use std::fmt::Debug;
use std::process::Output;


use glm;
use rand::{self, Rng};

mod camera_cer;
mod cer_mat;
mod cer_object;
mod image_cer;
mod ray;
mod scene_cer;

use crate::camera_cer::{Camera, SimpleCamera};
use crate::image_cer::{pixel, ImageWorks, PixRGB};

const MAX_RAY_DEPTH: u32 = 16;
const NUM_SAMPLES: usize = 32;

fn send_ray(ray: &ray::Ray, scene: &scene_cer::SceneCer, depth: u32, ray_buffer : &mut std::vec::Vec<(ray::Ray, f32)>, ri_stack : &mut std::vec::Vec<f32>) -> PixRGB<f32> {
    if depth < MAX_RAY_DEPTH {
            //max depth not reached    
        ray_buffer.clear();
        let mut int_query = cer_object::RayCerIntQuery::default();
        let  (mut hit_obj_i, mut hit_obj, mut hit_int_query) : (i32, Option<&dyn cer_object::CerSceneObject>, cer_object::RayCerIntQuery) = (-1, None, cer_object::RayCerIntQuery::default()); 
        
        for (obj_i, obj) in (0..).zip(scene.iter()){
            obj.ray_hit_query(ray, &mut int_query);
            if int_query.did_hit == true{
                //hit something
                if int_query.hit_t < 0.001 && int_query.other_t.unwrap_or(-1.) > int_query.hit_t{
                        let new_ray = ray::Ray::new(ray.origin + ray.dir * 0.001, ray.dir);
                        obj.ray_hit_query(&new_ray, &mut int_query);
                }
                if int_query.hit_t > 0.001{
                    if hit_obj.is_none() || hit_int_query.hit_t >= int_query.hit_t{
                        //Depth value of hit is less than previously hit object
                        hit_obj_i = obj_i;
                        hit_obj = Some(obj.as_ref());
                        hit_int_query = int_query.clone();
                    }
                }
            }
        }
        match hit_obj {
            Some(obj) => {
                let mut pix_val =  obj.get_mat().get_mat_response(ray, &hit_int_query, ray_buffer, ri_stack);
                //return pix_val;
                for (_count_i, add_ray) in (0..).zip(ray_buffer.clone().iter()){
                    pix_val = pix_val * send_ray(&add_ray.0, scene, depth + 1, ray_buffer, ri_stack) *  add_ray.1;
                }
                return pix_val;
            }
            None => ()
        }
    }
    //hit nothing or max depth reached
    let tt: f32 = 0.5 * (glm::normalize(ray.dir).y + 1.0);
    let color = glm::Vec3::new(1., 1., 1.) * (1.0 - tt) + glm::Vec3::new(0.5, 0.7, 1.0) * tt;
    return PixRGB::new(color.x, color.y, color.z);

}

fn main() {
    const WIDTH: u32 = 1920;
    const HEIGHT: u32 = 1080;
    let cam = camera_cer::SimpleCamera::new(1., WIDTH, HEIGHT);
    let mut img = image_cer::ImageCer::new(WIDTH as u16, HEIGHT as u16);
    let mut scene = scene_cer::SceneCer::default();

    let csphere = cer_object::CerSphere::new(
        1.0 as f32,
        glm::Vec3::new(2., 0., 2.),
        std::sync::Arc::new(cer_mat::DielectricMat::new(1.5)),
    );
    let csphere2 = cer_object::CerSphere::new(
        1 as f32,
        glm::Vec3::new(-2., 0., 2.),
        std::sync::Arc::new(cer_mat::MetalMat::new()),
    );
    let csphere3 = cer_object::CerSphere::new(
        0.9 as f32,
        glm::Vec3::new(2., 0.0, 2.),
        std::sync::Arc::new(cer_mat::DielectricMat::new(1.)),
    );
    let cplane = cer_object::CerPlane::new(
        1.,
        glm::Vec3::new(0., -1., 0.),
        std::sync::Arc::new(cer_mat::DiffuseMat::new(pixel(0.0, 0.83, 0.31))),
    );
    let cplanes = cer_object::CerSphere::new(
        100 as f32,
        glm::Vec3::new(0., 101., 2.),
        std::sync::Arc::new(cer_mat::DiffuseMat::new(pixel(0.2, 0.7, 0.2))),
    );

    let cspherem = cer_object::CerSphere::new(
        1 as f32,
        glm::Vec3::new(0., 0., 2.),
        std::sync::Arc::new(cer_mat::DiffuseMat::new(pixel(1.0,0.0,0.0)))
    );

    // let light_sphere  = cer_object::CerSphere::new(
    //     0.5,
    //     glm::vec3(0., -4., 0.),
    //     std::sync::Arc::new(cer_mat::EmmisionMat::new(pixel(1., 1., 1.), 5.0))
    // );

    scene.add_object(&csphere);
    scene.add_object(&csphere2);
    scene.add_object(&csphere3);
    scene.add_object(&cspherem);
    scene.add_object(&cplanes);
    // scene.add_object(&light_sphere);

    let perform_timer = std::time::Instant::now();

    let texel_size = glm::vec2(
        1. / (img.get_width() - 1) as f32,
        1. / (img.get_height() - 1) as f32,
    ) * 0.5;

    img.perform_for_each_mut(
        |pix, uv, args| -> () {
            let mut rand_gen = rand::thread_rng();
            for i in 0..NUM_SAMPLES {
                let curr_rand: f32 = rand_gen.gen_range(-100000..100000) as f32 / 100000.;
                let cam_ray = args.2.get_camera_ray(uv + (args.3 * curr_rand));
                let mut buf = std::vec::Vec::with_capacity(6);
                let mut ri_stack = vec![1.0];
                let additional_val = args.0(&cam_ray, args.1, 0,&mut buf, &mut ri_stack);
                *pix = PixRGB::new(
                    pix.r + additional_val.r,
                    pix.g + additional_val.g,
                    pix.b + additional_val.b,
                );
            }
            let sample_scale = 1.0 / NUM_SAMPLES as f32;
            *pix = PixRGB::new(
                (pix.r * sample_scale).sqrt(),
                (pix.g * sample_scale).sqrt(),
                (pix.b * sample_scale).sqrt(),
            );
        },
        (send_ray, &scene, &cam, texel_size),
    );

    println!(
        "Time taken (in ms) : {}",
        perform_timer.elapsed().as_millis()
    );

    let mut img_index: u64 = 1;
    for i in std::fs::read_dir("Resource/").unwrap() {
        match i {
            Ok(entry) => {
                match entry.metadata(){
                    Ok(metadata) => img_index += metadata.is_file() as u64,
                    Err(_err) => ()
                }
            },
            Err(_err) => (),
        }
    }
    println!("Resource/RayTraced{}.jpg", img_index);
    match img.save(&format!("Resource/RayTraced{}.jpg", img_index)) {
        Ok(()) => (),
        Err(err) => println!("{}", err),
    }
    println!("Hello, world!");
}
