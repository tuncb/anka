use clap::Parser;
use std::fs;
use std::path::Path;

#[derive(Parser, Debug)]
#[clap(author, version, about, long_about = None)]
struct Args {
    #[clap(short, long, value_parser)]
    filename: String,
}

fn main() {
    let args = Args::parse();
    println!("The filename given: {}", args.filename);

    let path = Path::new(&args.filename);

    if !Path::exists(path) {
        println!("Anka: File not found: {}", args.filename);
        return;
    }

    let code = fs::read_to_string(path);
    if code.is_err() {
        println!("Anka: Could not read file: {}", args.filename);
        return;
    }
    println!("{}", code.unwrap());
}
