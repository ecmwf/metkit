use metkit::{CodesHandle, MarsRequest};

fn main() -> Result<(), Box<dyn std::error::Error>> {
    eckit::init();

    // [codes]
    let mut handle = CodesHandle::from_sample("GRIB2")?;

    let edition: i64 = handle.get("edition")?;
    assert_eq!(edition, 2);

    handle.set("shortName", "t")?;
    handle.set("date", 20240101_i64)?;

    let values: Vec<f64> = handle.get("values")?;
    assert_eq!(values.len(), handle.value_count("values")?);
    handle.set("values", &values[..])?;

    assert!(handle.is_defined("centre")?);
    assert!(!handle.is_missing("centre")?);

    let bytes: &[u8] = handle.message_data()?;
    assert_eq!(bytes.len(), handle.message_size()?);

    let copy = CodesHandle::from_message(bytes)?;
    let name: String = copy.get("shortName")?;
    assert_eq!(name, "t");
    // [end codes]

    // [messages]
    let path = std::env::temp_dir().join("metkit-rust-example.grib");
    std::fs::write(&path, bytes)?;

    let (mut input, _length) = eckit::DataHandle::from_path(&path)?.open_for_read()?;
    for message in eckit::MessageReader::new(&mut input)? {
        let message = message?;
        let request = MarsRequest::try_from(&message)?;
        println!("{}", request.to_json()?);
    }
    // [end messages]
    Ok(())
}
