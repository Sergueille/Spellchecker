
use crate::vec2d::Vec2D;

// TODO: better support for strange characters such as œ, for instance oe and œ should have a distance that is < 1, but the distance is currently 2

/// Minimal value taken by insertion cost
pub const MIN_INSERTION_COST: f32 = 1.0;
/// Minimal value taken by deletion cost
pub const MIN_DELETION_COST: f32 = 1.0;

/// Cost of a substitution where a diacritic is ommited (e/é or c/ç)
/// Low value because generally the user forget it, and it doesn't change completely the meaning of the word
pub const DIACRITIC_SUBTITUTION_COST: f32 = 0.1;
pub const DIACRITICS_PAIRS: [[char; 2]; 15] = [
    ['é', 'e'],
    ['è', 'e'],
    ['ê', 'e'],
    ['ë', 'e'],
    ['â', 'a'],
    ['à', 'a'],
    ['ä', 'a'],
    ['î', 'i'],
    ['ï', 'i'],
    ['ù', 'u'],
    ['û', 'u'],
    ['ü', 'u'],
    ['ô', 'o'],
    ['ö', 'o'],
    ['ç', 'c'],
];

/// Cost of a substitution where letters are adjacent ok keyboard
pub const ADJACENT_SUBTITUTION_COST: f32 = 0.7;

/// Cost of a transposition
pub const BASE_TRANSPOSITION_COST: f32 = 1.1;
/// Cost of a transposition where letters are adjacent ok keyboard
pub const ADJACENT_TRANSPOSITION_COST: f32 = 1.0;

#[derive(Clone, Copy)]
struct MemoisedValue {
    pub val: Option<f32>,
    pub max_cost: f32,
}

/// Gets the edition distance between two strings
/// 
/// # Arguments
/// * `a` and `b`: the strings to compare
/// * if the cost exceeds this, just return None
/// 
/// Uses https://bmcbioinformatics.biomedcentral.com/articles/10.1186/s12859-019-2819-0, with basic recursive dynamic programming and variable costs
pub fn get_distance(a: &str, b: &str, max_cost: f32) -> Option<f32> {
    // Get characters
    let a_chars: Vec<char> = a.chars().collect();
    let b_chars: Vec<char> = b.chars().collect();

    // Create memoisation table
    let mut memo = Vec2D::new(a_chars.len() + 1, b_chars.len() + 1, MemoisedValue { val: None, max_cost: f32::INFINITY });

    let res = get_distance_aux(&mut memo, &a_chars, &b_chars, a_chars.len(), b_chars.len(), max_cost);

    // memo.print(|val| if val.val.is_some() { String::from("#") } else { String::from(".") });

    return if res == f32::INFINITY { None } else { Some(res) };
}

/// Returns f32::INFINITY if greater than max_cost
fn get_distance_aux(memo: &mut Vec2D<MemoisedValue>, a: &Vec<char>, b: &Vec<char>, i: usize, j: usize, max_cost: f32) -> f32 {
    let memoized = memo.get(i, j);

    if memoized.val.is_some() && memoized.max_cost >= max_cost {
        return memoized.val.unwrap(); // Value already computed
    }
    else { // Compute value
        let mut res: f32;

        if max_cost <= 0.0 {
            res = f32::INFINITY;
        }
        else if i == 0 { // Only additions
            res = 0.0;
            for k in 0..j {
                res += get_insertion_cost(b[k]);

                if res > max_cost { 
                    res = f32::INFINITY; 
                    break; 
                }
            }
        }
        else if j == 0 { // Only deletions
            res = 0.0;
            for k in 0..i {
                res += get_deletion_cost(a[k]);

                if res > max_cost { 
                    res = f32::INFINITY; 
                    break; 
                }
            }
        }
        else {
            let sub_cost = get_substitution_cost(a[i - 1], b[j - 1]);
            let sub_total_cost = sub_cost + get_distance_aux(memo, a, b, i - 1, j - 1, max_cost - sub_cost);
            
            let ins_cost = get_insertion_cost(b[j - 1]);
            let ins_total_cost = ins_cost + get_distance_aux(memo, a, b, i - 1, j, max_cost - ins_cost);

            let del_cost = get_deletion_cost(b[j - 1]);
            let del_total_cost = del_cost + get_distance_aux(memo, a, b, i, j - 1, max_cost - del_cost);

            let last_a = last_letter_id(a, b[j - 1], i - 1);
            let last_b = last_letter_id(b, a[i - 1], j - 1);

            let transp_total_cost = match (last_a, last_b) {
                (Some(la), Some(lb)) => {
                    let transp_cost = get_transposition_cost(a[i - 1], b[j - 1]);
                    let mut add_sum = 0.0;
                    let mut rem_sum = 0.0;

                    for k in (lb + 1)..(j - 1) {
                        add_sum += get_insertion_cost(b[k]);
                    }

                    for k in (la + 1)..(i - 1) {
                        rem_sum += get_deletion_cost(a[k]);
                    }

                    let base_cost = get_distance_aux(memo, a, b, la, lb, max_cost - add_sum - rem_sum - transp_cost);
                    base_cost + add_sum + rem_sum + transp_cost
                },
                _ => f32::INFINITY
            };

            res = f32::min(
                f32::min(sub_total_cost, ins_total_cost),
                f32::min(del_total_cost, transp_total_cost),
            );
        }
        
        if res >= max_cost {
            res = f32::INFINITY;
        }

        memo.set(i, j, MemoisedValue {
            val: Some(res),
            max_cost: max_cost,
        });

        return res;
    }
}


pub fn get_insertion_cost(_c: char) -> f32 {
    return 1.0;
}

pub fn get_deletion_cost(_c: char) -> f32 {
    return 1.0;
}

// Swap A with B
pub fn get_substitution_cost(a: char, b: char) -> f32 {
    if a == b {
        return 0.0;
    }
    else if are_characters_equal_ignoring_diacritics(a, b) {
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
pub fn get_transposition_cost(a: char, b: char) -> f32 {
    if a == b {
        return 0.0;
    }
    else if are_characters_adjacent_on_keyboard(a, b) {
        return ADJACENT_TRANSPOSITION_COST;
    }
    else {
        return BASE_TRANSPOSITION_COST;
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

pub fn are_characters_equal_ignoring_diacritics(a: char, b: char) -> bool {
    for pair in DIACRITICS_PAIRS {
        if (a == pair[0] && b == pair[1])
        || (b == pair[0] && a == pair[1]) {
            return true;
        }
    }

    return false;
}

/// FIXME: it's only for my keyboard
/// Returns true if letter key shares a border on the keyboard
pub fn are_characters_adjacent_on_keyboard(a: char, b: char) -> bool {
    let lines = ["azertyuiop^", "qsdfghjklmù", "<wxcvbn,;:!"];
    let is_middle_line_shifted_on_right = true;

    // Get letter position
    for i in 0..3 {
        let char_vec: Vec<char> = lines[i].chars().collect();
        let a_index = last_letter_id(&char_vec, a, char_vec.len());

        match a_index {
            Some(id) => {
                if id < char_vec.len() - 1 && char_vec[id + 1] == b { return true; }
                if id > 0 && char_vec[id - 1] == b { return true; }

                if is_middle_line_shifted_on_right {
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
