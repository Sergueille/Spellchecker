use std::fs;


mod distance;
mod vec2d;

#[derive(Clone, Copy)]
struct WordWithCost {
    pub word_id: usize,
    pub cost: f32,
}

fn main() {
    let args: Vec<String> = std::env::args().collect();

    let word = &args[1];
    let dict = read_words();

    let start_time = std::time::Instant::now();
    let mut skip_count = 0;

    // Compute distances
    let best_count = 5;
    let mut bests: Vec<WordWithCost> = Vec::with_capacity(best_count);
    
    for i in 0..dict.len() {
        // Get characters
        let word_len = word.chars().count();
        let other_len = dict[i].chars().count();

        // Determine a minimum cost quickly with length
        // It prevents calling get_distance unecessarily, and saves half of the time
        let min_cost = if word_len > other_len {
            (word_len - other_len) as f32 * distance::MIN_DELETION_COST
        } else {
            (other_len - word_len) as f32 * distance::MIN_INSERTION_COST
        };

        if bests.len() == best_count && min_cost >= bests[best_count - 1].cost {
            skip_count += 1;
            continue; // Not good enough!
        }

        let dist = distance::get_distance(word, &dict[i]);

        // Insert into bests
        if bests.len() == 0 {
            bests.push(WordWithCost {
                word_id: i,
                cost: dist,
            });
        }
        else {
            for j in (0..bests.len()).rev() {
                if bests[j].cost >= dist {
                    if j < bests.len() - 1 {
                        bests[j + 1] = bests[j];
                    }
                    else if bests.len() < best_count {
                        bests.push(bests[j]);
                    }
    
                    bests[j] = WordWithCost {
                        word_id: i,
                        cost: dist,
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


