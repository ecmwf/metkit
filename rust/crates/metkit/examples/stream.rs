use eckit::{MemoryStream, StreamWrite};
use metkit::{Expanded, MarsRequest};

fn main() -> metkit::Result<()> {
    eckit::init();

    // [stream]
    let request: MarsRequest<Expanded> =
        "retrieve, class=od, expver=0001, date=20240101, param=130".parse()?;

    let mut out = MemoryStream::writer();
    request.encode(&mut out)?;
    request.write_to(&mut out)?;
    let bytes = out.buffer()?.to_vec();

    let mut input = MemoryStream::reader(&bytes);
    let decoded = MarsRequest::decode(&mut input)?;
    assert_eq!(decoded.get("param")?, "130");
    assert!(decoded.matches(&request));
    // [end stream]
    Ok(())
}
