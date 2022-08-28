use std::sync::Arc;
use std::boxed::Box;

use crate::ray;
use crate::cer_mat;
use glm;

#[derive(Clone, Copy, Debug)]
pub struct RayCerIntQuery {
    pub did_hit: bool,
    pub hit_t: f32,
    pub other_t: Option<f32>,
    pub normal_int: glm::Vec3,
    pub coords_int: glm::Vec3,
}

impl Default for RayCerIntQuery {
    fn default() -> Self {
        Self {
            did_hit: false,
            hit_t: f32::NAN,
            other_t: None,
            normal_int: glm::Vec3::new(f32::NAN, f32::NAN, f32::NAN),
            coords_int: glm::Vec3::new(f32::NAN, f32::NAN, f32::NAN),
        }
    }
}

pub trait CerSceneObjectClone {
    fn cer_clone(&self) -> Box<dyn CerSceneObject>;
}
impl<T> CerSceneObjectClone for T
where
    T: 'static + CerSceneObject + Clone,
{
    fn cer_clone(&self) -> Box<dyn CerSceneObject> {
        Box::new(self.clone())
    }
}

pub trait CerSceneObject: CerSceneObjectClone + Send + Sync {
    fn ray_hit_query(&self, ray: &ray::Ray, int_query: &mut RayCerIntQuery);
    fn did_ray_hit(&self, ray: &ray::Ray) -> bool {
        let mut int_query = RayCerIntQuery::default();
        self.ray_hit_query(ray, &mut int_query);
        int_query.did_hit
    }
    fn get_mat(&self) -> Arc<dyn cer_mat::MatCer>;
}

#[derive(Clone, Debug)]
pub struct CerSphere {
    radius: f32,
    center: glm::Vec3,
    material : Arc<dyn cer_mat::MatCer>,
}

impl CerSphere {
    pub fn new(radius: f32, center: glm::Vec3, mat : Arc<dyn cer_mat::MatCer>) -> Self {
        CerSphere {
            radius: radius,
            center: center,
            material: mat
        }
    }
}

fn f32_min(a: f32, b: f32) -> f32 {
    a.min(b)
}

impl CerSceneObject for CerSphere {
    fn get_mat(&self) -> Arc<dyn cer_mat::MatCer> {
        self.material.clone()
    }
    fn ray_hit_query(&self, ray: &ray::Ray, int_query: &mut RayCerIntQuery) {
        let factor_a = glm::dot(ray.dir, ray.dir);
        let factor_b = 2. * glm::dot(ray.dir, ray.origin - self.center);
        let factor_c =
            glm::dot(ray.origin - self.center, ray.origin - self.center) - self.radius.powi(2);

        let intersection_discriminant = factor_b * factor_b - (4. * factor_a * factor_c);
        if intersection_discriminant < 0. {
            // ray completely misses sphere
            int_query.did_hit = false;
            return;
        }
        let sqrt_id = intersection_discriminant.sqrt();
        let (t_factor1, t_factor2) = (
            ((-factor_b + sqrt_id) / (2. * factor_a)),
            (-factor_b - sqrt_id) / (2. * factor_a),
        );

        // check if ray hits the sphere in front wrt its direction
        if t_factor1 < 0. {
            if t_factor2 < 0. {
                // both negative => t is behind ray origin wrt direction
                int_query.did_hit = false;
                return;
            } else {
                // 1 is neg, 2 is pos => 2 is only option
                int_query.hit_t = t_factor2;
                int_query.other_t = Some(t_factor1);
            }
        } else {
            if t_factor2 < 0. {
                // 1 is pos, 2 is neg => 1 is only option
                int_query.hit_t = t_factor1;
                int_query.other_t = Some(t_factor2);
            } else {
                // both positive => choose minimum (first intersection with surface)
                if t_factor1 < t_factor2{
                    int_query.hit_t = t_factor1;
                    int_query.other_t = Some(t_factor2);
                }
                else{
                    int_query.hit_t = t_factor2;
                    int_query.other_t = Some(t_factor1);
                }
            }
        }
        // ray hitting sphere is valid, calculate required info
        // hit_t and other_t already set
        int_query.did_hit = true;
        int_query.coords_int = ray.origin + ray.dir * int_query.hit_t;
        int_query.normal_int = glm::normalize(int_query.coords_int - self.center);

        let front_face = glm::dot(ray.dir, int_query.normal_int) < 0.;
        if self.radius.is_sign_negative(){
            int_query.normal_int = - int_query.normal_int;
        }
        else{
            if !front_face{
                int_query.normal_int = -int_query.normal_int;
            }
        }

    }

    fn did_ray_hit(&self, ray: &ray::Ray) -> bool {
        let factor_a = glm::dot(ray.dir, ray.dir);
        let factor_b = 2. * glm::dot(ray.dir, ray.origin - self.center);
        let factor_c =
            glm::dot(ray.origin - self.center, ray.origin - self.center) - self.radius.powi(2);

        let intersection_discriminant = factor_b * factor_b - (4. * factor_a * factor_c);
        let (t_factor1, t_factor2) = (
            ((-factor_b + intersection_discriminant) / (2. * factor_a)),
            (-factor_b - intersection_discriminant) / (2. * factor_a),
        );
        intersection_discriminant >= 0. && (t_factor1 >= 0. || t_factor2 >= 0.)
    }
}




#[derive(Clone, Debug)]
pub struct CerPlane {
    d: f32,
    normal: glm::Vec3,
    material : Arc<dyn cer_mat::MatCer>,
}

impl CerPlane {
    pub fn new(d: f32, normal: glm::Vec3, mat : Arc<dyn cer_mat::MatCer>) -> Self {
        CerPlane {
            d: d,
            normal: glm::normalize(normal),
            material: mat
        }
    }
}

impl CerSceneObject for CerPlane {
    fn get_mat(&self) -> Arc<dyn cer_mat::MatCer> {
        self.material.clone()
    }
    fn ray_hit_query(&self, ray: &ray::Ray, int_query: &mut RayCerIntQuery) {
        if glm::dot(ray.dir, self.normal) != 0.{
            let t_factor = (-self.d - glm::dot(ray.origin, self.normal))/glm::dot(ray.dir, self.normal);
            if t_factor >= 0.{
                int_query.did_hit = true;
                int_query.hit_t = t_factor;
                int_query.normal_int = self.normal;
                int_query.coords_int = ray.at(t_factor);
                return;
            }
        }
        int_query.did_hit = false;
    }
}
