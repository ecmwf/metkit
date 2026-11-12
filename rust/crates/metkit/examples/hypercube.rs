use metkit::{Expanded, HyperCube, MarsRequest};

fn main() -> metkit::Result<()> {
    eckit::init();

    // [cube]
    let request: MarsRequest<Expanded> = "retrieve, class=od, expver=0001, stream=oper, \
        type=an, date=20240101, time=0/12, levtype=pl, levelist=500/850, param=129/130"
        .parse()?;

    let mut cube = HyperCube::new(&request)?;
    assert_eq!(cube.size(), 8);

    let field: MarsRequest<Expanded> = "retrieve, class=od, expver=0001, stream=oper, \
        type=an, date=20240101, time=12, levtype=pl, levelist=850, param=130"
        .parse()?;

    assert!(cube.contains(&field)?);
    println!("{}", cube.field_ordinal(&field)?);

    assert!(cube.clear(&field)?);
    assert!(!cube.clear(&field)?);
    assert_eq!(cube.count_vacant(), 7);
    // [end cube]
    Ok(())
}
