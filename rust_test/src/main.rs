use std::fs;


mod distance;
mod vec2d;
mod buffer;
mod fast_dist;

#[derive(Clone, Copy)]
struct WordWithCost {
    pub word_id: usize,
    pub cost: f32,
}

#[derive(Clone, Copy)]
struct WordWithCosts {
    pub word_id: usize,
    pub cost: f32,
    pub fast_cost: f32,
}

pub const FAST_DIST_BUFFER_SIZE: usize = 2000;
pub const CANDIDATES_COUNT: usize = 10;

pub const MAX_ALLOWED_DISTANCE: f32 = 5.0;

fn main() {
    let args: Vec<String> = std::env::args().collect();

    let input = &args[1];

    let mut start_time = std::time::Instant::now();

    let dict = read_words();
    println!("Reading file took {}ms", start_time.elapsed().as_millis());

    let input_processed = fast_dist::process_input(input);

    benchmark_on_words("Fast dist length", &dict, |w| { fast_dist::fast_dist_len(&input_processed, w); });
    benchmark_on_words("Fast dist count", &dict, |w| { fast_dist::fast_dist_count(&input_processed, w); });
    benchmark_on_words("Fast dist presence", &dict, |w| { fast_dist::fast_dist_pres(&input_processed, w); });
    benchmark_on_words("Fast dist pairs count", &dict, |w| { fast_dist::fast_dist_pairs_count(&input_processed, w); });

    start_time = std::time::Instant::now();

    let mut fast_buffer = buffer::Buffer::<WordWithCost>::new(FAST_DIST_BUFFER_SIZE, &|a: &WordWithCost, b: &WordWithCost| { a.cost - b.cost });
    for i in 0..dict.len() {
        let dist = fast_dist::fast_dist_combined(&input_processed, &dict[i]);
        fast_buffer.insert(WordWithCost {
            word_id: i,
            cost: dist,
        });
    }

    println!("Fast distances: {}", start_time.elapsed().as_millis());

    calculate_nearest(input, &fast_buffer, 2000, &dict);
    calculate_nearest(input, &fast_buffer, 1000, &dict);
    calculate_nearest(input, &fast_buffer, 500, &dict);

    println!("----");
    println!("All words (can take time):");
    start_time = std::time::Instant::now();

    let mut real_buffer = buffer::Buffer::<WordWithCost>::new(CANDIDATES_COUNT, &|a: &WordWithCost, b: &WordWithCost| { a.cost - b.cost });

    for i in 0..dict.len() {
        let dist = distance::get_distance(&dict[i], input);

        if dist.is_some() {
            real_buffer.insert(WordWithCost {
                word_id: i,
                cost: dist.unwrap(),
            });
        }
    }

    println!("{}ms", start_time.elapsed().as_millis());

    for i in 0..real_buffer.len() {
        println!(
            "{}\t{}\t({})", 
            dict[real_buffer.get(i).word_id], 
            real_buffer.get(i).cost, 
            fast_dist::fast_dist_combined(&input_processed, &dict[real_buffer.get(i).word_id])
        );
    }

    return;

    /*
    let mut skip_count = 0;

    // Compute distances
    let best_count = 3;
    let mut bests: Vec<WordWithCost> = Vec::with_capacity(best_count);
    
    for i in 0..dict.len() {
        // Get lengths
        let word_len = input.chars().count();
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
            distance::get_distance(input, &dict[i], f32::INFINITY)
        }
        else {
            distance::get_distance(input, &dict[i], max_dist)
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

    */
}

fn calculate_nearest(input: &str, fast_buffer: &buffer::Buffer<WordWithCost>, buffer_limit: usize, dict: &Vec<String>) {
    let mut real_buffer = buffer::Buffer::<WordWithCosts>::new(CANDIDATES_COUNT, &|a: &WordWithCosts, b: &WordWithCosts| { a.cost - b.cost });

    let start_time = std::time::Instant::now();

    for i in 0..buffer_limit {
        let dist = distance::get_distance(&dict[fast_buffer.get(i).word_id], input);

        if dist.is_some() {
            real_buffer.insert(WordWithCosts {
                word_id: fast_buffer.get(i).word_id,
                cost: dist.unwrap(),
                fast_cost: fast_buffer.get(i).cost,
            });
        }
    }

    println!("{} words in fast buffer: {}ms", buffer_limit, start_time.elapsed().as_millis());

    for i in 0..real_buffer.len() {
        println!("{}\t{}\t({})", dict[real_buffer.get(i).word_id], real_buffer.get(i).cost, real_buffer.get(i).fast_cost);
    }
}

fn benchmark_on_words<F: Fn(&str)>(label: &str, dict: &Vec<String>, f: F) {
    let start_time = std::time::Instant::now();
    for i in 0..dict.len() {
        f(&dict[i]);
    }

    println!("{}: {}ms", label, start_time.elapsed().as_millis());
}

// TEST: reads french_words.txt
fn read_words() -> Vec<String> {
    let text = String::from_utf8(
        fs::read("../french_words.txt").expect("Failed to read file")
    ).expect("Encoding error");

    return text.split("\n").into_iter().map(|s| s.to_owned()).collect();
}


