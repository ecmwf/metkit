use metkit::{Expanded, MarsRequest};

fn main() -> metkit::Result<()> {
    eckit::init();

    let mut request = MarsRequest::new("retrieve");
    request.set("class", "od");
    request.set("date", "-1");
    request.set("step", ["0", "6", "12"]);
    let expanded = request.expand(true, false)?;
    println!("{expanded}");

    let parsed: MarsRequest<Expanded> = "retrieve, class=od, date=-1, param=t".parse()?;
    println!("{}", parsed.get("param")?);
    Ok(())
}
