use std::fmt::Debug;

use crate::image_cer::PixRGB;
use crate::ray;
use crate::{cer_object, image_cer};
use glm::Vector3;
use rand::Rng;
pub trait MatCer: Send + Sync + Debug {
    fn get_mat_response(
        &self,
        ray: &ray::Ray,
        int_query: &cer_object::RayCerIntQuery,
        add_sample_rays: &mut std::vec::Vec<(ray::Ray, f32)>,
        ri_stack : &mut std::vec::Vec<f32>,
    ) -> PixRGB<f32>;
}

#[derive(Clone, Copy, Debug)]
pub struct SimpleMat {
    color: PixRGB<f32>,
}

impl SimpleMat {
    pub fn new(color: PixRGB<f32>) -> Self {
        Self { color: color }
    }
}

impl MatCer for SimpleMat {
    fn get_mat_response(
        &self,
        ray: &ray::Ray,
        int_query: &cer_object::RayCerIntQuery,
        add_sample_rays: &mut std::vec::Vec<(ray::Ray, f32)>,
        ri_stack : &mut std::vec::Vec<f32>,
    ) -> PixRGB<f32> {
        self.color * (glm::dot(int_query.normal_int, glm::vec3(-0.0, -0.0, -0.75)).max(0.) + 0.12)
    }
}

#[derive(Clone, Copy, Debug)]
pub struct DiffuseMat {
    color: PixRGB<f32>,
}

impl DiffuseMat {
    pub fn new(color: PixRGB<f32>) -> Self {
        Self { color: color }
    }
}

fn rand_in_unit_sphere() -> Vector3<f32> {
    let mut rng = rand::thread_rng();
    let mut rand_vec = glm::vec3(
        rng.gen_range(-1.0..1.0),
        rng.gen_range(-1.0..1.0),
        rng.gen_range(-1.0..1.0),
    );
    while glm::length(rand_vec) >= 1. {
        rand_vec = glm::vec3(
            rng.gen_range(-1.0..1.0),
            rng.gen_range(-1.0..1.0),
            rng.gen_range(-1.0..1.0),
        );
    }
    rand_vec
}

fn rand_unit_vec() -> Vector3<f32> {
    glm::normalize(rand_in_unit_sphere())
}

impl MatCer for DiffuseMat {
    fn get_mat_response(
        &self,
        ray: &ray::Ray,
        int_query: &cer_object::RayCerIntQuery,
        add_sample_rays: &mut std::vec::Vec<(ray::Ray, f32)>,
        ri_stack : &mut std::vec::Vec<f32>,
    ) -> PixRGB<f32> {
        add_sample_rays.push((
            ray::Ray::new(int_query.coords_int, int_query.normal_int + rand_unit_vec()),
            0.5,
        ));
        self.color
    }
}

#[derive(Clone, Copy, Debug)]
pub struct MetalMat {}

impl MetalMat {
    pub fn new() -> Self {
        Self {}
    }
}

fn reflect(v: glm::Vec3, n: glm::Vec3) -> glm::Vec3 {
    v - n * 2. * glm::dot(v, n)
}

impl MatCer for MetalMat {
    fn get_mat_response(
        &self,
        ray: &ray::Ray,
        int_query: &cer_object::RayCerIntQuery,
        add_sample_rays: &mut std::vec::Vec<(ray::Ray, f32)>,
        ri_stack : &mut std::vec::Vec<f32>,
    ) -> PixRGB<f32> {
        add_sample_rays.push((
            ray::Ray::new(
                int_query.coords_int,
                rand_in_unit_sphere() * 0.2 + reflect(ray.dir, int_query.normal_int),
            ),
            0.8,
        ));
        PixRGB::new(1., 1., 1.)
    }
}

#[derive(Clone, Copy, Debug)]
pub struct DielectricMat {
    refractive_i: f32,
}

impl DielectricMat {
    pub fn new(refractive_i: f32) -> Self {
        Self {
            refractive_i: refractive_i,
        }
    }
}

pub fn refract(ray_dir: glm::Vec3, norm: glm::Vec3, refractive_ratio: f32) -> glm::Vec3 {
    let cos_theta = glm::dot(-ray_dir, norm).min(1.0);
    let out_perp = (ray_dir + norm * cos_theta) * refractive_ratio;
    let out_parl = norm * (1.0 - glm::dot(out_perp, out_perp)).abs().sqrt() * -1.;
    out_perp + out_parl
}

pub fn reflectance(cosine : f32, refractive_index : f32) -> f32{
    //Schlick Approximation
    let r0 = ((1. - refractive_index) / (1. + refractive_index)).powi(2);
    r0 + (1. - r0) * (1. - cosine).powi(5)
}

impl MatCer for DielectricMat {
    fn get_mat_response(
        &self,
        ray: &ray::Ray,
        int_query: &cer_object::RayCerIntQuery,
        add_sample_rays: &mut std::vec::Vec<(ray::Ray, f32)>,
        ri_stack : &mut std::vec::Vec<f32>,
    ) -> PixRGB<f32> {
        let refraction_ratio: f32;
        match int_query.other_t {
            Some(other_t) => {
                if (other_t * int_query.hit_t).is_sign_negative(){
                    //ray exiting material
                    ri_stack.pop();
                    refraction_ratio = self.refractive_i / ri_stack.last().unwrap_or(&1.0);

                }
                else{
                    //ray entering material
                    refraction_ratio = ri_stack.last().unwrap_or(&1.0) / self.refractive_i;
                    ri_stack.push(self.refractive_i);
                }
            }
            None => return PixRGB::new(0., 0., 0.),
        }

        // if refraction_ratio == 1.{
        //     add_sample_rays.push((*ray, 1.0));
        //     return PixRGB::new(1.0,1.0,1.0);
        // }

        let normed_rd = glm::normalize(ray.dir);
        let cos_theta = glm::dot(-normed_rd, int_query.normal_int).min(1.0);
        let sin_theta = (1.0 - cos_theta*cos_theta).sqrt();
        let cannot_refract = sin_theta * refraction_ratio > 1.0;
        if cannot_refract ||
            reflectance(cos_theta, refraction_ratio) > rand::thread_rng().gen_range(0.0..=1.0){
            add_sample_rays.push((ray::Ray::new(
                int_query.coords_int, reflect(ray.dir, int_query.normal_int)),
            1.));
        }
        else{
            let refract_dir = refract(normed_rd, int_query.normal_int, refraction_ratio);
            add_sample_rays.push((
                ray::Ray::new(
                    int_query.coords_int + refract_dir * 0.0001,
                    refract_dir),
                    1.)
            );
        }
        PixRGB::new(1., 1., 1.)
    }
}



#[derive(Clone, Copy, Debug)]
pub struct EmmisionMat {
    color: PixRGB<f32>,
    emissivity : f32,
}

impl EmmisionMat {
    pub fn new(color: PixRGB<f32>, emisivity : f32) -> Self {
        Self { color: color, emissivity : emisivity }
    }
}

impl MatCer for EmmisionMat {
    fn get_mat_response(
        &self,
        ray: &ray::Ray,
        int_query: &cer_object::RayCerIntQuery,
        add_sample_rays: &mut std::vec::Vec<(ray::Ray, f32)>,
        ri_stack : &mut std::vec::Vec<f32>,
    ) -> PixRGB<f32> {
        self.color * self.emissivity
    }
}
