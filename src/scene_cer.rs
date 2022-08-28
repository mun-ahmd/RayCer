use crate::cer_object::{CerSceneObject};
use crate::ray;

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
    pub fn iter(&self) -> std::slice::Iter<'_,Box<dyn CerSceneObject>>{
        self.cer_objects.iter()
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