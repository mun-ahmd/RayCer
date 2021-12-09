use core::{num, prelude};
use crossbeam as cb;
use image::{
    io::Reader as ImageReader, DynamicImage, EncodableLayout, GenericImage, GenericImageView,
};
use itertools::Itertools;
use std::{
    env::VarError,
    fmt::Error,
    io::Read,
    os::unix::thread,
    sync::{Arc, Mutex},
    vec::Vec,
};

pub trait ImageWorks
where
    Self: Sized,
{
    type PixelType: Sized + Sync + Send + Copy;
    fn get_width(&self) -> u32;
    fn get_height(&self) -> u32;

    fn get_pixel(&self, x_coord: u16, y_coord: u16) -> Option<Self::PixelType>;
    fn get_mut_pixel(&mut self, x_coord: u16, y_coord: u16) -> Option<&mut Self::PixelType>;

    fn set_pixel(&mut self, x_coord: u16, y_coord: u16, pixel_val: Self::PixelType);

    fn perform_for_each(self: Arc<Self>, action: fn(Self::PixelType, glm::Vec2));
    fn perform_for_each_mut<Targs: Sized + Send + Sync + Clone>(
        self: &mut Self,
        action: fn(&mut Self::PixelType, glm::Vec2, &Targs),
        action_arg: Targs,
    );

    fn new_from_bytes(img_width: u16, img_height: u16, img_data: std::vec::Vec<u8>) -> Self;
    fn new_load(filepath: &str) -> Option<Self>;
    fn save(&self, filepath: &str) -> Result<(), std::io::Error>;
}

#[derive(Clone, Copy, Debug, Default)]
pub struct PixRGB<T> {
    pub r: T,
    pub g: T,
    pub b: T,
}

impl<T> PixRGB<T> {
    pub fn new(r: T, g: T, b: T) -> PixRGB<T> {
        PixRGB { r: r, g: g, b: b }
    }
}

#[derive(Clone, Debug)]
pub struct ImageCer {
    image_data: Vec<PixRGB<f32>>,
    width: u16,
    height: u16,
}

impl ImageCer {
    pub fn new(img_width: u16, img_height: u16) -> Self {
        Self {
            image_data: vec![
                PixRGB {
                    r: 0.,
                    g: 0.,
                    b: 0.
                };
                img_width as usize * img_height as usize
            ],
            width: img_width,
            height: img_height,
        }
    }
}

impl ImageWorks for ImageCer {
    type PixelType = PixRGB<f32>;
    fn get_width(&self) -> u32 {
        self.width as u32
    }

    fn get_height(&self) -> u32 {
        self.height as u32
    }

    fn get_pixel(&self, x_coord: u16, y_coord: u16) -> Option<PixRGB<f32>> {
        Some(
            *self
                .image_data
                .get((y_coord as usize * self.width as usize + x_coord as usize) as usize)?,
        )
    }

    fn get_mut_pixel(&mut self, x_coord: u16, y_coord: u16) -> Option<&mut PixRGB<f32>> {
        Some(
            self.image_data
                .get_mut((y_coord as usize * self.width as usize + x_coord as usize) as usize)?,
        )
    }

    fn set_pixel(&mut self, x_coord: u16, y_coord: u16, pixel_val: PixRGB<f32>) {
        match self
            .image_data
            .get_mut((y_coord as usize * self.width as usize + x_coord as usize) as usize)
        {
            Some(pix) => *pix = pixel_val,
            None => (),
        }
    }

    fn perform_for_each(self: Arc<Self>, action: fn(PixRGB<f32>, glm::Vec2) -> ()) {
        let num_threads = num_cpus::get();
        let mut threads: std::vec::Vec<std::thread::JoinHandle<()>> =
            std::vec::Vec::with_capacity(1);
        let width_per_thread = self.width as usize / num_threads;
        for thread_index in 0..(num_threads - 1) {
            let arced_self = Arc::clone(&self);
            threads.push(std::thread::spawn(move || {
                for x in (thread_index * width_per_thread)..((thread_index + 1) * width_per_thread)
                {
                    for y in 0..arced_self.height {
                        action(
                            arced_self.get_pixel(x as u16, y).unwrap(),
                            glm::Vec2::new(
                                x as f32 / (arced_self.width - 1) as f32,
                                y as f32 / (arced_self.height - 1) as f32,
                            ),
                        );
                    }
                }
            }));
        }
        for x in (num_threads - 1) * width_per_thread..self.width as usize {
            for y in 0..self.height {
                action(
                    self.get_pixel(x as u16, y).unwrap(),
                    glm::Vec2::new(
                        x as f32 / (self.width - 1) as f32,
                        y as f32 / (self.height - 1) as f32,
                    ),
                )
            }
        }
        for join_handle in threads {
            join_handle.join().unwrap();
        }
    }

    fn perform_for_each_mut<Targs: Sized + Send + Sync + Clone>(
        self: &mut Self,
        action: fn(&mut Self::PixelType, glm::Vec2, &Targs),
        action_arg: Targs,
    ) {
        let actiona = Arc::new(action);
        let num_threads = num_cpus::get();
        println!("Using {} threads!", num_threads);

        let width_per_thread = self.width as usize / num_threads;
        let height_per_thread = self.height as usize / num_threads;

        let img_width = self.width;
        let img_height = self.height;

        let mut thread_index = 0;

        cb::scope(|scope| {
            let woww = &action_arg;
            for image_chunk in self
                .image_data
                .chunks_mut(height_per_thread * self.width as usize)
            {
                let mut index_within_chunk: usize = 0;
                if thread_index == num_threads - 1 {
                    for y in thread_index * height_per_thread..img_height as usize {
                        for x in 0..img_width {
                            action(
                                image_chunk.get_mut(index_within_chunk).unwrap(),
                                glm::Vec2::new(
                                    x as f32 / (img_width - 1) as f32,
                                    y as f32 / (img_height - 1) as f32,
                                ),woww
                            );
                            index_within_chunk += 1;
                        }
                    }
                } else {
                    scope.spawn(move |_| {
                        for y in (thread_index * height_per_thread)
                            ..((thread_index + 1) * height_per_thread)
                        {
                            for x in 0..img_width {
                                action(
                                    image_chunk.get_mut(index_within_chunk).unwrap(),
                                    glm::Vec2::new(
                                        x as f32 / (img_width - 1) as f32,
                                        y as f32 / (img_height - 1) as f32,
                                    ),woww
                                );
                                index_within_chunk += 1;
                            }
                        }
                    });
                }
                //due to move, thread index is copied so no issues here
                thread_index += 1;
            }
        })
        .unwrap();
    }

    fn new_from_bytes(img_width: u16, img_height: u16, img_data: std::vec::Vec<u8>) -> Self {
        assert_eq!(
            img_data.len(),
            (img_height as usize * img_width as usize * 3) as usize
        );
        let mut image_fin = Self {
            image_data: std::vec::Vec::with_capacity(img_data.len()),
            width: img_width,
            height: img_height,
        };
        for chunk in &img_data.into_iter().chunks(3) {
            let pix_vals: Vec<u8> = chunk.collect();
            image_fin.image_data.push(PixRGB {
                r: pix_vals[0] as f32 / 255.,
                g: pix_vals[1] as f32 / 255.,
                b: pix_vals[2] as f32 / 255.,
            });
        }
        image_fin
    }

    fn new_load(filepath: &str) -> Option<Self> {
        let loaded_img = ImageReader::open(filepath).ok()?.decode().ok()?;
        Some(Self::new_from_bytes(
            loaded_img.width() as u16,
            loaded_img.height() as u16,
            loaded_img.as_rgb8()?.as_bytes().to_vec(),
        ))
    }

    fn save(&self, filepath: &str) -> Result<(), std::io::Error> {
        let mut img_to_save = image::DynamicImage::new_rgb8(self.width as u32, self.height as u32);

        for y in 0..self.height {
            for x in 0..self.width {
                let curr_pixel =
                    self.get_pixel(x, self.height - 1 - y)
                        .ok_or(std::io::Error::new(
                            std::io::ErrorKind::Other,
                            "invalid bounds",
                        ))?;
                img_to_save.put_pixel(
                    x as u32,
                    y as u32,
                    image::Rgba([
                        (curr_pixel.r * 255.) as u8,
                        (curr_pixel.g * 255.) as u8,
                        (curr_pixel.b * 255.) as u8,
                        255,
                    ]),
                );
            }
        }
        match img_to_save.save(filepath) {
            Ok(()) => return Ok(()),
            Err(err) => {
                return Err(std::io::Error::new(
                    std::io::ErrorKind::Other,
                    err.to_string(),
                ))
            }
        }
    }
}
