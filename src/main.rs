use std::fs;


mod distance;
mod vec2d;

#[derive(Clone, Copy)]
struct WordWithCost {
    pub word_id: usize,
    pub cost: f32,
}

pub const MAX_ALLOWED_DISTANCE: f32 = 3.0;

fn main() {
    let args: Vec<String> = std::env::args().collect();

    /*
    let test = distance::get_distance("supplémentaire", "retourner", f32::INFINITY);
    println!("{:?}", test);
    return;
    */

    let word = &args[1];
    let dict = read_words();

    let start_time = std::time::Instant::now();
    let mut skip_count = 0;

    // Compute distances
    let best_count = 3;
    let mut bests: Vec<WordWithCost> = Vec::with_capacity(best_count);
    
    for i in 0..dict.len() {
        // Get lengths
        let word_len = word.chars().count();
        let other_len = dict[i].chars().count();

        let max_dist = if bests.len() == best_count { f32::min(MAX_ALLOWED_DISTANCE, bests[best_count - 1].cost) } else { MAX_ALLOWED_DISTANCE };

        // Determine a minimum cost quickly with length
        // It prevents calling get_distance unecessarily, and saves half of the time
        let min_cost = if word_len > other_len {
            (word_len - other_len) as f32 * distance::MIN_DELETION_COST
        } else {
            (other_len - word_len) as f32 * distance::MIN_INSERTION_COST
        };

        if bests.len() == best_count && min_cost >= max_dist {
            skip_count += 1;
            continue; // Not good enough!
        }

        // Compute the actual distance
        let dist = if bests.len() < best_count {
            distance::get_distance(word, &dict[i], f32::INFINITY)
        }
        else {
            distance::get_distance(word, &dict[i], max_dist)
        };

        let float_dist = match dist {
            Some(d) => d,
            None => f32::INFINITY,
        };

        // Insert into bests
        if bests.len() == 0 {
            bests.push(WordWithCost {
                word_id: i,
                cost: float_dist,
            });
        }
        else {
            for j in (0..bests.len()).rev() {
                if bests[j].cost >= float_dist {
                    if j < bests.len() - 1 {
                        bests[j + 1] = bests[j];
                    }
                    else if bests.len() < best_count {
                        bests.push(bests[j]);
                    }
    
                    bests[j] = WordWithCost {
                        word_id: i,
                        cost: float_dist,
                    };
                }
                else { break; }
            }
        }
    }

    println!("{}ms, {}ns/word, {} skipped", 
        start_time.elapsed().as_millis(),
        start_time.elapsed().as_nanos() / dict.len() as u128, 
        skip_count
    );

    // Show result
    for i in 0..best_count {
        println!("{}\t{}", dict[bests[i].word_id], bests[i].cost);
    }
}


// TEST: reads french_words.txt
fn read_words() -> Vec<String> {
    let text = String::from_utf8(
        fs::read("french_words.txt").expect("Failed to read file")
    ).expect("Encoding error");

    return text.split("\n").into_iter().map(|s| s.to_owned()).collect();
}


