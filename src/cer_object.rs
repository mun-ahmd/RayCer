use std::{borrow::Borrow, cell::Cell};

use glm;

use crate::ray;

#[derive(Clone, Copy, Debug)]
pub struct RayCerIntQuery {
    pub did_hit: bool,
    pub normal_int: glm::Vec3,
    pub coords_int: glm::Vec3,
}

impl Default for RayCerIntQuery {
    fn default() -> Self {
        Self {
            did_hit: false,
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

pub trait CerSceneObject: CerSceneObjectClone + Send + Sync{
    fn ray_hit_query(&self, ray: &ray::Ray, int_query: &mut RayCerIntQuery);
    fn did_ray_hit(&self, ray: &ray::Ray) -> bool {
        let mut int_query = RayCerIntQuery::default();
        self.ray_hit_query(ray, &mut int_query);
        int_query.did_hit
    }
}

#[derive(Default)]
pub struct SceneCer{
    cer_objects: std::vec::Vec<std::boxed::Box<dyn CerSceneObject>>,
}
impl SceneCer {
    pub fn add_object(&mut self, obj: &dyn CerSceneObject) {
        self.cer_objects.push(obj.cer_clone());
    }
    pub fn test_ray(&self, ray: ray::Ray) -> bool {
        for obj in self.cer_objects.iter() {
            if obj.did_ray_hit(&ray) {
                return true;
            }
        }
        false
    }
}

impl AsRef<SceneCer> for SceneCer{
    fn as_ref(&self) -> &SceneCer {
        self
    }
}

impl Clone for SceneCer {
    fn clone(&self) -> Self {
        let mut cloned_objs: std::vec::Vec<Box<dyn CerSceneObject>> =
            std::vec::Vec::with_capacity(self.cer_objects.len());
        for ele in self.cer_objects.iter() {
            cloned_objs.push(ele.cer_clone());
        }
        SceneCer {
            cer_objects: cloned_objs,
        }
    }
}
#[derive(Clone, Copy, Debug)]
pub struct CerSphere {
    radius: f32,
    center: glm::Vec3,
}

impl CerSphere {
    pub fn new(radius: f32, center: glm::Vec3) -> CerSphere {
        CerSphere {
            radius: radius,
            center: center,
        }
    }
}

fn f32_min(a: f32, b: f32) -> f32 {
    if a < b {
        a
    } else {
        b
    }
}

impl CerSceneObject for CerSphere {
    fn ray_hit_query(&self, ray: &ray::Ray, int_query: &mut RayCerIntQuery) {
        let factor_a = glm::dot(ray.dir, ray.dir);
        let factor_b = 2. * glm::dot(ray.dir, ray.origin - self.center);
        let factor_c =
            glm::dot(ray.origin - self.center, ray.origin - self.center) - self.radius.powi(2);

        let intersection_discriminant = factor_b * factor_b - (4. * factor_a * factor_c);
        let (t_factor1, t_factor2) = (
            ((-factor_b + intersection_discriminant) / (2. * factor_a)),
            (-factor_b - intersection_discriminant) / (2. * factor_a),
        );
        if intersection_discriminant < 0. {
            // ray completely misses sphere
            int_query.did_hit = false;
            return;
        }

        let t_factor: f32;
        // check if ray hits the sphere in front wrt its direction
        if t_factor1 < 0. {
            if t_factor2 < 0. {
                // both negative => t is behind ray origin wrt direction
                int_query.did_hit = false;
                return;
            } else {
                // 1 is neg, 2 is pos => 2 is only option
                t_factor = t_factor2;
            }
        } else {
            if t_factor2 < 0. {
                // 1 is pos, 2 is neg => 1 is only option
                t_factor = t_factor1;
            } else {
                // both positive => choose minimum (first intersection with surface)
                t_factor = f32_min(t_factor1, t_factor2);
            }
        }
        // ray hitting sphere is valid, calculate required info
        int_query.did_hit = true;
        int_query.coords_int = ray.origin + ray.dir * t_factor;
        int_query.normal_int = glm::normalize(int_query.coords_int - self.center);
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
