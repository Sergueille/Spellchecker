#![allow(dead_code)]

// Keeps the `max_size` smallest values sorted from smallest to biggest
// Compare should return a - b
pub struct Buffer<'a, T> {
    pub max_size: usize,
    data: Vec<T>,
    pub compare: &'a dyn Fn(&T, &T) -> f32,
}

pub struct BufferIterator<'a, T> {
    pos: usize,
    buffer: &'a Buffer<'a, T>
}

impl<T> Buffer<'_, T> {
    pub fn new(size: usize, compare: &'static dyn Fn(&T, &T) -> f32) -> Buffer<T> {
        return Buffer {
            max_size: size,
            data: Vec::with_capacity(size),
            compare: compare,
        };
    }

    pub fn insert(&mut self, value: T) {
        let mut i = self.max_size - 1;

        if self.data.len() < self.max_size {
            i = self.data.len();
            self.data.push(value);
        }
        else {
            if (self.compare)(&value, &self.data[i]) < 0.0 {
                self.data[i] = value;
            }
            else {
                return;
            }
        }

        while i > 0 && (self.compare)(&self.data[i], &self.data[i - 1]) < 0.0 {
            self.data.swap(i, i - 1);
            i -= 1;
        }
    }

    pub fn get(&self, i: usize) -> &T {
        return &self.data[i];
    }

    pub fn len(&self) -> usize {
        return self.data.len();
    }

    pub fn clear(&mut self) {
        self.data.clear();
    }
    
}

impl<'a, T> std::iter::Iterator for BufferIterator<'a, T> {
    type Item = &'a T;

    fn next(&mut self) -> Option<Self::Item> {
        
        if self.pos < self.buffer.len() {
            self.pos += 1;
            return Some(self.buffer.get(self.pos - 1));
        }
        else {
            return None;
        }
    }
}

impl<'a, T> std::iter::IntoIterator for &'a Buffer<'a, T> {
    type Item = &'a T;
    type IntoIter = BufferIterator<'a, T>;

    fn into_iter(self) -> Self::IntoIter {
        return BufferIterator {
            buffer: self,
            pos: 0,
        };
    }
}

