use std::io::Read;

use metkit::{Expanded, MarsRequest, MarsRequestHandle};

// [hermes]
fn retrieve(request: &MarsRequest<Expanded>, database: &eckit::Config) -> metkit::Result<Vec<u8>> {
    let (mut reading, estimated_length) =
        MarsRequestHandle::new(request, database)?.open_for_read()?;

    let mut data = Vec::with_capacity(usize::try_from(estimated_length).unwrap_or(0));
    reading
        .read_to_end(&mut data)
        .map_err(|e| metkit::Error::Other(e.to_string()))?;
    Ok(data)
}
// [end hermes]

fn main() -> metkit::Result<()> {
    eckit::init();
    metkit::initialize(&[("client", "metkit-rust-example"), ("version", "0.1")]);

    let Some(config_path) = std::env::args().nth(1) else {
        println!("usage: hermes <database.yaml>");
        return Ok(());
    };
    let database = eckit::Config::from_path(&config_path)?;
    let request: MarsRequest<Expanded> = "retrieve, class=od, date=-1, param=t".parse()?;

    let data = retrieve(&request, &database)?;
    println!("{}", data.len());
    Ok(())
}
