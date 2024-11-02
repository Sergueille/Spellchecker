use std::fs;
use colored::Colorize;

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

pub const FAST_DIST_BUFFER_SIZE: usize = 500;
pub const CANDIDATES_COUNT: usize = 10;

pub const MAX_ALLOWED_DISTANCE: f32 = 5.0;

fn main() {
    let args: Vec<String> = std::env::args().collect();

    if args.len() != 3 {
        println!("USAGE: <command> <word>\nCommands: file, bench, test-fast");
        return;
    }

    let command = &args[1];
    let input = &args[2];

    if command == "file" {
        file(input)
    }
    else if command == "bench" {
        benchmark(input);
    }
    else if command == "test-fast" {
        test_fast_words(input);
    }
}

fn file(input: &str) {
    let start_time = std::time::Instant::now();

    let dict = read_words();
    let input_processed = fast_dist::process_input(input);

    let mut fast_buffer = buffer::Buffer::<WordWithCost>::new(FAST_DIST_BUFFER_SIZE, &|a: &WordWithCost, b: &WordWithCost| { a.cost - b.cost });
    let mut real_buffer = buffer::Buffer::<WordWithCost>::new(CANDIDATES_COUNT, &|a: &WordWithCost, b: &WordWithCost| { a.cost - b.cost });

    for i in 0..dict.len() {
        let dist = fast_dist::fast_dist_combined(&input_processed, &dict[i]);
        fast_buffer.insert(WordWithCost {
            word_id: i,
            cost: dist,
        });
    }

    for i in 0..FAST_DIST_BUFFER_SIZE {
        let dist = distance::get_distance(&dict[fast_buffer.get(i).word_id], input);

        if dist.is_some() {
            real_buffer.insert(WordWithCost {
                word_id: fast_buffer.get(i).word_id,
                cost: dist.unwrap(),
            });
        }
    }

    let mut text = String::with_capacity(CANDIDATES_COUNT * 20);
    for i in 0..CANDIDATES_COUNT {
        text.push_str(&dict[real_buffer.get(i).word_id]);
        text.push('\0');
    }
    text.push('\0');
    fs::write(".out", text).unwrap();

    println!("Created `.out` file. Total time elapsed: {}ms", start_time.elapsed().as_millis());
}

fn benchmark(input: &str) {
    let mut start_time = std::time::Instant::now();

    let dict = read_words();
    println!("Reading file: {}ms", start_time.elapsed().as_millis());

    let input_processed = fast_dist::process_input(input);

    benchmark_on_words("Fast dist length", &dict, |w| { fast_dist::fast_dist_len(&input_processed, w); });
    benchmark_on_words("Fast dist count", &dict, |w| { fast_dist::fast_dist_count(&input_processed, w); });
    benchmark_on_words("Fast dist presence", &dict, |w| { fast_dist::fast_dist_pres(&input_processed, w); });
    benchmark_on_words("Fast dist pairs count", &dict, |w| { fast_dist::fast_dist_pairs_count(&input_processed, w); });

    start_time = std::time::Instant::now();

    let fast_buffer = get_fast_words(input, 2000, &dict);

    println!("Fast distances: {}", start_time.elapsed().as_millis());

    calculate_nearest(input, &fast_buffer, 2000, &dict);
    calculate_nearest(input, &fast_buffer, 1000, &dict);
    calculate_nearest(input, &fast_buffer, 500, &dict);

    println!("----");
    println!("All words (can take time):");
    start_time = std::time::Instant::now();

    let real_buffer = get_all_words_slow(input, &mut dict.iter());

    println!("{}ms", start_time.elapsed().as_millis());

    for i in 0..real_buffer.len() {
        println!(
            "{}\t{}\t({})", 
            dict[real_buffer.get(i).word_id], 
            real_buffer.get(i).cost, 
            fast_dist::fast_dist_combined(&input_processed, &dict[real_buffer.get(i).word_id])
        );
    }
}

fn test_fast_words(file_name: &str) {
    let file_text = String::from_utf8(
        fs::read(file_name).expect("Failed to read file")
    ).expect("Encoding error");
    
    let tests = file_text.split('\n');

    let dict = read_words();

    println!("For each proposition made by the slow algorithm (left to right), the position of it in the fast algorithm's rank {}     Worst of all since beginning", "(worst since beginning)".bright_black());
    
    let buffer_sizes = vec![1000, 2000, 5000];
    let mut ok_counts = vec![vec![0; CANDIDATES_COUNT]; buffer_sizes.len()];

    let mut worse_of_worst: Option<usize> = Some(0);
    let mut worsts: Vec<Option<usize>> = vec![Some(0); CANDIDATES_COUNT];

    let mut total_count = 0;

    for word in tests {
        let fast_size = 20000;
        let fast = get_fast_words(word, fast_size, &dict);
        let slow = get_all_words_slow(word, &mut (&fast).into_iter().map(|word_with_cost| &dict[word_with_cost.word_id] ));
        
        print!("{: <20} ", word);
        
        let mut worst: Option<usize> = Some(0);
        for i in 0..slow.len() {
            let slow_proposition = fast.get(slow.get(i).word_id).word_id;

            let mut found = false;
            for j in 0..fast.len() {
                if fast.get(j).word_id == slow_proposition {

                    for k in 0..buffer_sizes.len() {
                        if j < buffer_sizes[k] {
                            ok_counts[k][i] += 1;
                        }
                    }

                    if is_worse(Some(j), worst) {
                        worst = Some(j);
                    }
                    
                    if is_worse(Some(j), worsts[i]) {
                        worsts[i] = Some(j);
                    }

                    print!("{: <20} ", format!("{} {}", j, format!("({})", &show_worst(worsts[i])).bright_black()));
                    found = true;
                }
            }

            if !found {
                print!("> 20k                ", );
            }
        }

        if is_worse(worst, worse_of_worst) {
            worse_of_worst = worst;
        }

        print!("{}\n", show_worst(worse_of_worst));

        total_count += 1;
    }

    println!("Proportions of words that were selected by the fast algorithm");

    for k in 0..buffer_sizes.len() {
        print!("{: <20} ", buffer_sizes[k]);

        for i in 0..CANDIDATES_COUNT {
            print!("{: <20} ", ok_counts[k][i] as f64 / total_count as f64);
        }

        print!("\n");
    }
}

fn get_all_words_slow<'a>(input: &str, dict: &mut dyn std::iter::Iterator<Item = &String>) -> buffer::Buffer<'a, WordWithCost> {
    let mut real_buffer = buffer::Buffer::<WordWithCost>::new(CANDIDATES_COUNT, &|a: &WordWithCost, b: &WordWithCost| { a.cost - b.cost });

    for (i, word) in dict.enumerate() {
        let dist = distance::get_distance(&word, input);

        if dist.is_some() {
            real_buffer.insert(WordWithCost {
                word_id: i,
                cost: dist.unwrap(),
            });
        }
    }

    return real_buffer;
}

fn get_fast_words<'a>(input: &str, buffer_size: usize, all_words: &'a Vec<String>) -> buffer::Buffer<'a, WordWithCost> {
    let mut fast_buffer = buffer::Buffer::<WordWithCost>::new(buffer_size, &|a: &WordWithCost, b: &WordWithCost| { a.cost - b.cost });
    let input_processed = fast_dist::process_input(&input);

    for i in 0..all_words.len() {
        let dist = fast_dist::fast_dist_combined(&input_processed, &all_words[i]);
        fast_buffer.insert(WordWithCost {
            word_id: i,
            cost: dist,
        });
    }

    return fast_buffer;
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

// Is a worse than b?
fn is_worse(a: Option<usize>, b: Option<usize>) -> bool {
    return a.is_none() || a.is_some() && (
        b.is_some() && (a.unwrap() > b.unwrap())
    )
}

fn show_worst(w: Option<usize>) -> String {
    match w {
        Some(v) => v.to_string(),
        None => String::from("> 20k"),
    }
}

