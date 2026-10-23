use metkit::{Error, Expanded, MarsRequest};

fn main() {
    eckit::init();

    // [errors]
    let result: metkit::Result<MarsRequest<Expanded>> = "retrieve, class=od, foo=bar".parse();

    match result {
        Ok(request) => println!("{request}"),
        Err(Error::Eckit(eckit::Error::UserError(msg))) => {
            println!("rejected by the MARS language: {msg}");
        }
        Err(Error::Metkit(metkit_sys::Error::CodesException(msg))) => {
            println!("ecCodes failed: {msg}");
        }
        Err(other) => println!("{other}"),
    }
    // [end errors]
}
