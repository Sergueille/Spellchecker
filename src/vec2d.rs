
pub struct Vec2D<T> {
    data: Vec<T>,
    x_size: usize,
    y_size: usize,
}

#[allow(dead_code)]
impl<T> Vec2D<T> 
where T: Clone {
    pub fn new(x_size: usize, y_size: usize, val: T) -> Vec2D<T> {
        let mut data: Vec<T> = Vec::with_capacity(x_size * y_size);
        for _ in 0..(x_size * y_size) {
            data.push(val.clone());
        }

        return Vec2D::<T> {
            data,
            x_size,
            y_size
        };
    }

    pub fn get(&self, x: usize, y: usize) -> T {
        if x >= self.x_size || y >= self.y_size {
            panic!("Coordinates out of bounds for Vec2D::get : ({}, {}), the size is ({}, {})", x, y, self.x_size, self.y_size);
        }

        return self.data[x + y * self.x_size].clone();
    }

    pub fn set(&mut self, x: usize, y: usize, val: T) {
        if x >= self.x_size || y >= self.y_size {
            panic!("Coordinates out of bounds for Vec2D::set : ({}, {}), the size is ({}, {})", x, y, self.x_size, self.y_size);
        }

        return self.data[x + y * self.x_size] = val;
    }

    pub fn print(&self, printfn: fn(T) -> String) {
        for y in 0..self.y_size {
            for x in  0..self.x_size {
                print!("{} ", printfn(self.get(x, y)));
            }

            println!();
        }
    }
}
