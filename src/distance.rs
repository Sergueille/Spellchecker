
use crate::vec2d::Vec2D;
extern crate unidecode;

// TODO: better support for strange characters such as œ, for instance oe and œ should have a distance that is < 1, but the distance is currently 2

/// Minimal value taken by insertion cost
pub const MIN_INSERTION_COST: f32 = 1.0;
/// Minimal value taken by deletion cost
pub const MIN_DELETION_COST: f32 = 1.0;

/// Cost of a substitution where a diacritic is ommited (e/é or c/ç)
/// Low value because generally the user forget it, and it doesn't change completely the meaning of the word
pub const DIACRITIC_SUBTITUTION_COST: f32 = 0.1;

/// Cost of a substitution where letters are adjacent ok keyboard
pub const ADJACENT_SUBTITUTION_COST: f32 = 0.5;

/// Cost of a transposition where letters are adjacent ok keyboard
pub const ADJACENT_TRANSPOSITION_COST: f32 = 0.5;

/// Gets the edition distance between two strings
/// 
/// Uses https://bmcbioinformatics.biomedcentral.com/articles/10.1186/s12859-019-2819-0, with basic recursive dynamic programming and variable costs
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

// Swap A with B
pub fn get_substitution_cost(a: char, b: char) -> f32 {
    if a == b {
        return 0.0;
    }
    else if are_characters_equal_with_diacritics(a, b) {
        return DIACRITIC_SUBTITUTION_COST;
    }
    else if are_characters_adjacent_on_keyboard(a, b) {
        return ADJACENT_SUBTITUTION_COST;
    }
    else {
        return 1.0;
    }
}

// NOTE: Make sure 2(transposition_cost) ≥ (insetion_cost)+(deletion_cost) for all letters
// FIXME: this isn't respected!
pub fn get_transposition_cost(a: char, b: char) -> f32 {
    if a == b {
        return 0.0;
    }
    else if are_characters_adjacent_on_keyboard(a, b) {
        return ADJACENT_TRANSPOSITION_COST;
    }
    else {
        return 1.0;
    }
}

/// Returs the last index of c in s\[0:max_i\], -1 if not found
fn last_letter_id(s: &Vec<char>, c: char, max_i: usize) -> Option<usize> {
    for i in (0..max_i).rev() {
        if s[i] == c {
            return Some(i);
        }
    }

    return None;
}

pub fn are_characters_equal_with_diacritics(a: char, b: char) -> bool {
    let a_decoded = unidecode::unidecode(&String::from(a));
    let b_decoded = unidecode::unidecode(&String::from(b));

    return (a_decoded.len() == 1 && a_decoded.chars().next().unwrap() == b)
        || (b_decoded.len() == 1 && b_decoded.chars().next().unwrap() == a);
}

/// FIXME: it's only for my keyboard
/// Returns true if letter key shares a border on the keyboard
pub fn are_characters_adjacent_on_keyboard(a: char, b: char) -> bool {
    let lines = ["azertyuiop^", "qsdfghjklmù", "<wxcvbn,;:!"];
    let is_middle_line_shifted_of_right = true;

    // Get letter position
    for i in 0..3 {
        let char_vec: Vec<char> = lines[i].chars().collect();
        let a_index = last_letter_id(&char_vec, a, char_vec.len());

        match a_index {
            Some(id) => {
                if id < char_vec.len() - 1 && char_vec[id + 1] == b { return true; }
                if id > 0 && char_vec[id - 1] == b { return true; }

                if is_middle_line_shifted_of_right {
                    let shift_up: i32 = [0, 0, -1][i];
                    let shift_down: i32 = [-1, 0, 0][i];

                    let int_id = id as i32;
                    if i > 0 && int_id + shift_up >= 0 && lines[i - 1].chars().nth((int_id + shift_up) as usize) == Some(b) { return true; }
                    if i > 0 && int_id + shift_up >= 0 && lines[i - 1].chars().nth((int_id + shift_up + 1) as usize) == Some(b) { return true; }
                    if i < 2 && int_id + shift_down >= 0 && lines[i + 1].chars().nth((int_id + shift_down) as usize) == Some(b) { return true; }
                    if i < 2 && int_id + shift_down >= 0 && lines[i + 1].chars().nth((int_id + shift_down + 1) as usize) == Some(b) { return true; }
                }
                else {
                    if i > 0 && lines[i - 1].chars().nth(id) == Some(b) { return true; }
                    if i < 2 && lines[i + 1].chars().nth(id) == Some(b) { return true; }
                }
            },
            None => {},
        }
    } 

    return false;
}
