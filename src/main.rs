use std::fmt::Debug;
use std::process::Output;

use glm;

mod camera_cer;
mod cer_object;
mod image_cer;
mod ray;

use crate::camera_cer::{Camera, SimpleCamera};
use crate::image_cer::{ImageWorks, PixRGB};

#[derive(Clone, Copy, Debug)]
struct Sphere {
    radius: f32,
    center: glm::Vec3,
}



impl Sphere {
    pub fn new(radius: f32, center: glm::Vec3) -> Sphere {
        Sphere {
            radius: radius,
            center: center,
        }
    }
    pub fn does_ray_hit(&self, ray: ray::Ray) -> bool {
        let factor_a = glm::dot(ray.dir, ray.dir);
        let factor_b = 2. * glm::dot(ray.dir, ray.origin - self.center);
        let factor_c =
            glm::dot(ray.origin - self.center, ray.origin - self.center) - self.radius.powi(2);

        let intersection_discriminant = factor_b * factor_b - (4. * factor_a * factor_c);

        if intersection_discriminant >= 0. {
            true
        } else {
            false
        }
    }
}

fn main() {
    let cam = camera_cer::SimpleCamera::new(1.0, 1024, 720);
    let mut img = image_cer::ImageCer::new(1024, 720);
    let mut scene = cer_object::SceneCer::default();
    let csphere = cer_object::CerSphere::new(1.5, glm::Vec3::new(0., -0.5, 2.));
    let csphere2 = cer_object::CerSphere::new(1.5, glm::Vec3::new(0., 0., 2.));
    scene.add_object(&csphere);
    scene.add_object(&csphere2);

    let x = (1,"ss"); 

    let (vw, vh) = (img.get_width(), img.get_height());

    let perform_timer = std::time::Instant::now();    

    img.perform_for_each_mut(|pix,uv,args| -> (){
        let scene = &args.0;
        let cam = &args.1;
        let curr_ray = cam.get_camera_ray(uv);
        if scene.test_ray(curr_ray) {
            *pix = PixRGB::new(1., 1., 1.);
        } else {
            let mut color = glm::Vec3::new(0., 0., 0.);
            let tt: f32 = 0.5 * (glm::normalize(curr_ray.dir).y + 1.0);
            color =
                glm::Vec3::new(1., 1., 1.) * (1.0 - tt) + glm::Vec3::new(0.5, 0.7, 1.0) * tt;
            *pix =  PixRGB::new(color.x, color.y, color.z);
        }
    },(Box::new(scene),cam));
    println!("doing perform_for_each_mut!");

    img.perform_for_each_mut(|pix,uv,()| {
        if pix.r != 1.{
            *pix = PixRGB::new(uv.x, uv.y, 0.5);
        };
    },());

    println!("completed perform_for_each_mut!");

    println!("Time taken (in ms) : {}",perform_timer.elapsed().as_micros());
    // 96ms with 8 threads
    // 470ms with 1 thread
    // that is almost 5 times less with 8 times the threads
    // 148ms with 4 threads
    // 4.8 ms with 8 threads in release config

    let mut img_index: u64 = 0;
    for i in std::fs::read_dir("Resource/").unwrap() {
        match i {
            Ok(entry) => (),
            Err(err) => continue,
        }
        img_index += 1;
    }

    match img.save(&format!("Resource/RayTraced{}.jpg", img_index)) {
        Ok(()) => (),
        Err(err) => println!("{}", err),
    }
    println!("Hello, world!");
}
