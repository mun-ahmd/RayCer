use glm::Vec3;

#[derive(Clone, Copy, Debug)]
pub struct Ray {
    pub origin: Vec3,
    pub dir: Vec3,
}

impl Ray {
    pub fn new(ray_origin: Vec3, ray_dir: Vec3) -> Self {
        Self {
            origin: ray_origin,
            dir: ray_dir,
        }
    }
    
    pub fn at(self, t_factor : f32) -> Vec3{
        self.origin + self.dir * t_factor
    }
}

impl Default for Ray {
    fn default() -> Self {
        Ray {
            origin: Vec3::new(0., 0., 0.),
            dir: Vec3::new(0., 0., 1.),
        }
    }
}
