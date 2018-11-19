from sqlalchemy import create_engine, text

db = create_engine("postgres://ecmwf_ro:ecmwf_ro@db-products-dev-00.ecmwf.int/products")
TYPES = text("select distinct stream, type, levtype from fields")
PARAMS = text("select distinct param from fields where stream=? and type=? and levtype=?")


for stream, type, levtype in db.execute(TYPES):
    print(list(db.execute(PARAMS, dict(stream=stream, type=type, levtype=levtype))))
