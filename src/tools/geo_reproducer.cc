
#include "eckit/runtime/Main.h"
#include "eckit/geo/Grid.h"

int main() {
    const char* args[] = {"mars2grib", ""};
    eckit::Main::initialise(1, const_cast<char**>(args));

    const std::string marsGrid = "O80";

    const auto grid = eckit::geo::GridFactory::build(eckit::spec::Custom{{"grid", marsGrid}});
    const auto type = grid->type();

    std::cout << "grid type = " << type << std::endl;

    return 0;
}
