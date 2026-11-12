use metkit::{Expanded, MarsRequest, Raw};

fn main() -> metkit::Result<()> {
    eckit::init();

    // [build]
    let mut request = MarsRequest::new("retrieve");
    request.set("class", "od");
    request.set("expver", "0001");
    request.set("date", "20240101");
    request.set("param", ["129", "130"]);
    request.set("step", 6_i64);
    request.unset("expver");
    // [end build]

    // [read]
    assert_eq!(request.verb(), "retrieve");
    assert!(request.has("param"));
    assert_eq!(request.get("class")?, "od");
    assert_eq!(request.values("param")?, ["129", "130"]);
    assert_eq!(request.params().len(), 4);
    assert_eq!(request.count(), 2);
    println!("{request}");
    println!("{}", request.to_json()?);
    // [end read]

    // [expand]
    let expanded: MarsRequest<Expanded> = request.expand(true, false)?;
    assert_eq!(expanded.get("date")?, "20240101");
    assert_eq!(expanded.get("type")?, "an");
    // [end expand]

    // [parse]
    let parsed = metkit::parse("retrieve, class=od, date=-1\nlist, class=od", false)?;
    assert_eq!(parsed.count(), 2);
    for request in &parsed {
        let request = request?;
        println!("{} {}", request.verb(), request.count());
    }
    let all: Vec<MarsRequest<Expanded>> = parsed.to_vec()?;
    assert_eq!(all[1].verb(), "list");
    // [end parse]

    // [tokenize]
    let raw = metkit::tokenize("environ, user=alice")?;
    let environ: MarsRequest<Raw> = raw.at(0)?;
    assert_eq!(environ.get("user")?, "alice");
    // [end tokenize]

    // [trusted]
    let mut trusted = MarsRequest::<Expanded>::from_trusted(
        "retrieve, class=od, expver=0001, type=an, date=20240101, param=130",
    )?;
    trusted.set_trusted("expver", "0002");
    // [end trusted]

    // [edit]
    let mut edited = expanded.into_raw();
    edited.set("param", "131");
    let expanded = edited.expand(true, false)?;
    assert_eq!(expanded.get("param")?, "131");
    // [end edit]
    Ok(())
}
