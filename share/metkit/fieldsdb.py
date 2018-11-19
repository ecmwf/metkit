from sqlalchemy import create_engine, text

db = create_engine("postgres://ecmwf_ro:ecmwf_ro@db-products-dev-00.ecmwf.int/products")
TYPES = text("select distinct stream, type, levtype from fields")
PARAMS = text("select distinct param from fields where stream=:stream and type=:type and levtype=:levtype")


for stream, type, levtype in db.execute(TYPES):
    print('-'.join([stream, type, levtype]))
    params = list(db.execute(PARAMS, dict(stream=stream, type=type, levtype=levtype)))
    try:
        print("  %s" % sorted([int(x[0]) for x in params]))
    except:
        print("  %s" % params,)
