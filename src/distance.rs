
use crate::vec2d::Vec2D;

/// Gets the edition distance between two strings
/// 
/// Uses https://bmcbioinformatics.biomedcentral.com/articles/10.1186/s12859-019-2819-0, with basic recursive dynamic programming
pub fn get_distance(a: &str, b: &str) -> f32 {
    // Get characters
    let a_chars: Vec<char> = a.chars().collect();
    let b_chars: Vec<char> = b.chars().collect();

    // Create memoisation table
    let mut memo = Vec2D::new(a_chars.len() + 1, b_chars.len() + 1, None);

    return get_distance_aux(&mut memo, &a_chars, &b_chars, a_chars.len(), b_chars.len());
}

pub fn get_distance_aux(memo: &mut Vec2D<Option<f32>>, a: &Vec<char>, b: &Vec<char>, i: usize, j: usize) -> f32 {
    match memo.get(i, j) {
        Some(val) => return val, // Value already computed
        None => { // Compute value
            let mut res: f32;

            if i == 0 { // Only additions
                res = 0.0;
                for k in 0..j {
                    res += get_insertion_cost(b[k]);
                }
            }
            else if j == 0 { // Only deletions
                res = 0.0;
                for k in 0..i {
                    res += get_deletion_cost(a[k]);
                }
            }
            else {
                let substitution_cost = get_distance_aux(memo, a, b, i - 1, j - 1)
                                      + get_substitution_cost(a[i - 1], b[j - 1]);
                
                let insertiton_cost = get_distance_aux(memo, a, b, i - 1, j)
                                    + get_insertion_cost(b[j - 1]);

                let deletion_cost = get_distance_aux(memo, a, b, i, j - 1)
                                  + get_deletion_cost(a[i - 1]);

                let last_a = last_letter_id(a, b[j - 1], i - 1);
                let last_b = last_letter_id(b, a[i - 1], j - 1);
                
                let transposition_cost = match (last_a, last_b) {
                    (Some(la), Some(lb)) => {
                        let base_cost = get_distance_aux(memo, a, b, la, lb);
                        let mut add_sum = 0.0;
                        let mut rem_sum = 0.0;

                        for k in (lb + 1)..(j - 1) {
                            add_sum += get_insertion_cost(b[k]);
                        }

                        for k in (la + 1)..(i - 1) {
                            rem_sum += get_deletion_cost(a[k]);
                        }

                        base_cost + add_sum + rem_sum + get_transposition_cost(a[i - 1], b[j - 1])
                    },
                    _ => f32::INFINITY
                };


                res = f32::min(
                    f32::min(substitution_cost, insertiton_cost),
                    f32::min(deletion_cost, transposition_cost),
                );
            }

            memo.set(i, j, Some(res));
            return res;
        },
    }
}


pub fn get_insertion_cost(c: char) -> f32 {
    return 1.0;
}

pub fn get_deletion_cost(c: char) -> f32 {
    return 1.0;
}

pub fn get_substitution_cost(a: char, b: char) -> f32 {
    if a == b {
        return 0.0;
    }
    else {
        return 1.0;
    }
}

// NOTE: Make sure 2(transposition_cost) ≥ (insetion_cost)+(deletion_cost) for all letters
pub fn get_transposition_cost(a: char, b: char) -> f32 {
    if a == b {
        return 0.0;
    }
    else {
        return 1.0;
    }
}

/// Returs the last index of c in s[0:max_i], -1 if not found
fn last_letter_id(s: &Vec<char>, c: char, max_i: usize) -> Option<usize> {
    for i in (0..max_i).rev() {
        if s[i] == c {
            return Some(i);
        }
    }

    return None;
}
