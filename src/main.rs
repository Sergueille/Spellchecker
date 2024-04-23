use std::fs;


mod distance;
mod vec2d;

fn main() {
    let args: Vec<String> = std::env::args().collect();

    let word = &args[1];
    let dict = read_words();

    // Compute distances
    let mut bests = Vec::with_capacity(dict.len());
    for i in 0..dict.len() {
        let dist = distance::get_distance(word, &dict[i]);
        bests.push((dist, &dict[i]));
    }

    // Sort by distance
    bests.sort_by(|(a, _), (b, _)| f32::total_cmp(a, b));

    // Show result
    for i in 0..5 {
        let (d, w) = bests[i];
        println!("{}\t{}", w, d);
    }
}


// TEST: reads french_words.txt
fn read_words() -> Vec<String> {
    let text = String::from_utf8(
        fs::read("french_words.txt").expect("Failed to read file")
    ).expect("Encoding error");

    return text.split("\n").into_iter().map(|s| s.to_owned()).collect();
}


