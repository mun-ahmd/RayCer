use glm::Vec3;

use crate::ray;

pub trait Camera {
    fn get_camera_ray(&self, uv: glm::Vec2) -> ray::Ray;
}

#[derive(Clone, Debug)]
pub struct SimpleCamera {
    front: Vec3,
    up: Vec3,
    right: Vec3,
    pos: Vec3,
    focal_len: f64,
    aspect_ratio: f64,
}

impl SimpleCamera {
    pub fn new(focal_length: f64, canvas_width: u32, canvas_height: u32) -> Self {
        Self {
            front: Vec3::new(0., 0., 1.),
            up: Vec3::new(0., 1., 0.),
            right: Vec3::new(1., 0., 0.),
            pos: Vec3::new(0., 0., 0.),
            focal_len: focal_length,
            aspect_ratio: canvas_width as f64 / canvas_height as f64,
        }
    }
}

unsafe impl Send for SimpleCamera{
    
}

impl AsRef<SimpleCamera> for SimpleCamera{
    fn as_ref(&self) -> &SimpleCamera {
        self
    }
}

impl Camera for SimpleCamera {
    fn get_camera_ray(&self, uv: glm::Vec2) -> ray::Ray {
        let (viewport_w, viewport_h) = (2.0 * self.aspect_ratio as f32, 2. as f32);
        let lower_left_corner =
            self.pos - Vec3::new(viewport_w / 2., viewport_h / 2., self.focal_len as f32);
        ray::Ray::new(
            self.pos,
            lower_left_corner + Vec3::new(uv.x * viewport_w, uv.y * viewport_h, 0.),
        )
    }
}
