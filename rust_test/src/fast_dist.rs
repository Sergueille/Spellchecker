use crate::distance;


pub const INSERTION_DELETION_COST: f32 = 1.0;
pub const TRANSPOSITION_COST: f32 = 1.2;
pub const SUBSTITUTION_COST: f32 = 0.9;

// TODO: support for non ASCII characters
// -> should have little impact on final distances, but can be problematic if character is encoded on many bytes
// -> now considering one byte at a time, with a-z mapped to 0-25 and other codes mapped to 26


// Things I tried to make things faster
// -> for pairs, count 27 pars in one u64, with 2bytes/pair (600ms -> 200ms)
// -> for pairs, preallocate table (not faster)
// -> for pairs, use [u64; 27] instead of vec (210ms -> 190ms)

#[allow(dead_code)]
pub struct Input<'a> {
    word: &'a str,
    pair_table: [u64; 27],
    presence_table: u32,
    letter_table: Vec<u32>,
    len: u32,
}

pub fn process_input<'a>(w: &'a str) -> Input<'a> {
    return Input {
        word: w, 
        pair_table: get_pair_table(w),
        letter_table: get_table(w),
        presence_table: get_presence_table(w),
        len: w.chars().count() as u32,
    };
}

pub fn fast_dist_combined(a: &Input, b: &str) -> f32 {
    let c = fast_dist_count(a, b);
    let l = fast_dist_len(a, b);
    let p = fast_dist_pairs_count(a, b);

    return p as f32 * TRANSPOSITION_COST 
         + l as f32 * (6.0 * INSERTION_DELETION_COST - 3.0 * SUBSTITUTION_COST - TRANSPOSITION_COST)
         + c as f32 * (3.0 * SUBSTITUTION_COST - 2.0 * TRANSPOSITION_COST);
}

pub fn fast_dist_pres(a: &Input, b: &str) -> u32 {
    let b_c = get_presence_table(b);

    let diff = a.presence_table ^ b_c;

    let mut res = 0;

    for i in 0..27 {
        res += (diff >> i) & 1;
    }

    return res as u32;
}

pub fn fast_dist_len(a: &Input, b: &str) -> u32 {
    return a.len.abs_diff(b.chars().count() as u32);
}

pub fn fast_dist_count(a: &Input, b: &str) -> u32 {
    let table_b: Vec<u32> = get_table(b);

    let mut res = 0;

    for i in 0..27 {
        res += a.letter_table[i].abs_diff(table_b[i]);
    }

    return res;
}

pub fn fast_dist_pairs_count(a: &Input, b: &str) -> u32 {
    let table_b: [u64; 27] = get_pair_table(b);

    let mut res: u32 = 0;

    for i in 0..27 {
        for j in 0..27 {
            res += ((table_b[i] >> (j * 2)) & 0b11).abs_diff((a.pair_table[i] >> (j * 2)) & 0b11) as u32;
            // res += ((table_b[i] >> (j * 2)) & 0b11) as u32;
        }
    }

    return res;
}

fn get_table(a: &str) -> Vec<u32> {
    let mut res: Vec<u32> = Vec::with_capacity(27);

    for _ in 0..27 {
        res.push(0);
    }

    for c in a.chars() {
        let cc = c.to_ascii_lowercase();
        
        if cc.is_ascii() && cc >= 'a' && cc <= 'z' {
            res[cc as usize - 'a' as usize] += 1;
        }
        else {
            res[26] += 1;
        }
    }

    return res;
}

fn get_pair_table(a: &str) -> [u64; 27] {
    let mut res: [u64; 27] = [0; 27];

    let mut prev = '\0';

    for c in a.chars() {
        if prev != '\0' { 
            res[get_char_code(c)] += 1 << (get_char_code(prev) * 2);
        }

        prev = c;
    }

    return res;
}

fn get_presence_table(a: &str) -> u32 {
    let mut res = 0;

    for c in a.chars() {
        res |= 1 << get_char_code(c);
    }

    return res;
}

fn get_char_code(a: char) -> usize {
    if a.is_ascii() {
        let c = a.to_ascii_lowercase();
    
        if c >= 'a' && c <= 'z' {
            return c as usize - 'a' as usize;
        }
        else {
            return 26;
        }
    }
    else {
        let mut c = a.to_lowercase().next().unwrap();

        for pair in distance::DIACRITICS_PAIRS {
            if pair[0] == c {
                c = pair[1]
            }
        }

        if c.is_ascii() && c >= 'a' && c <= 'z' {
            return c as usize - 'a' as usize;
        }
        else {
            return 26;
        }
    }
}


