#include <fmt/core.h>
#include <stdio.h>

#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

const std::string WHITESPACE = " \n\r\t\f\v";

std::string ltrim(const std::string &line) {
    size_t start = line.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : line.substr(start);
}

std::string rtrim(const std::string &line) {
    size_t end = line.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : line.substr(0, end + 1);
}

std::string trim(const std::string &line) { return rtrim(ltrim(line)); }

struct Coords {
    double x;
    double y;
};

typedef int (*Dist)(Coords, Coords);

int dist_one(Coords v, Coords u) { return 1; }

int dist_euc2d(Coords v, Coords u) {
    double dx = v.x - u.x;
    double dy = v.y - u.y;
    return (sqrt(dx * dx + dy * dy) + 0.5);
}

int dist_att(Coords v, Coords u) {
    double xd = v.x - u.x;
    double yd = v.y - u.y;
    double rij = sqrt((xd * xd + yd * yd) / 10.0);
    int tij = (rij + 0.5);
    return tij + (tij < rij);
}

int dist_ceil2d(Coords v, Coords u) {
    double dx = v.x - u.x;
    double dy = v.y - u.y;
    return std::ceil(sqrt(dx * dx + dy * dy));
}

class Instance {
private:
    std::string name;
    std::string dist_type;
    std::vector<Coords> coords;
    Dist d;

public:
    Instance(std::string file_path) {
        std::ifstream infile(file_path);
        std::string line;

        int dim = 0;
        int coord_to_read = 0;
        while (std::getline(infile, line)) {

            if (coord_to_read > 0) {
                coord_to_read -= 1;
                std::stringstream line_stream(line);
                int id;
                double x, y;
                line_stream >> id >> x >> y;
                coords[id - 1] = Coords{x, y};
            } else {
                auto pos = line.find(":");

                std::string token = trim(line.substr(0, pos));
                std::string value = pos >= line.size() ? "" : trim(line.substr(pos + 1));

                if (token == "NAME") {
                    name = std::move(value);
                    fmt::print("Name: {}\n", name);
                } else if (token == "DIMENSION") {
                    dim = std::stoi(value);
                    coords.resize(dim);
                    fmt::print("Dimension: {}\n", value);
                } else if (token == "EDGE_WEIGHT_TYPE") {
                    fmt::print("Distance function: {}\n", value);
                    dist_type = value;
                    if (dist_type == "EUC_2D") {
                        d = dist_euc2d;
                    } else if (dist_type == "ATT") {
                        d = dist_att;
                    } else if (dist_type == "CEIL_2D") {
                        d = dist_ceil2d;
                    } else {
                        fmt::print(stderr, "Distance function \"{}\" not supported\n", dist_type);
                    }
                } else if (token == "NODE_COORD_TYPE") {
                    fmt::print("Coordinates type: {}\n", value);
                } else if (token == "NODE_COORD_SECTION") {
                    if (dim < 0) {
                        fmt::print(stderr, "Dimension not read yet!");
                        abort();
                    }
                    coord_to_read = dim;
                    fmt::print("Coords section:\n");
                } else if (token == "DEPOT_SECTION") {
                    fmt::print("Depot section: {}\n", value);
                } else {
                    fmt::print(stderr, "Keyword ignored {}\n", token);
                }
            }
        }
    }

    auto &get_name() const { return name; }
    auto &get_dist_type() const { return dist_type; }
    auto size() const { return coords.size(); }
    auto get_x(int i) { return coords[i].x; }
    auto get_y(int i) { return coords[i].y; }

    auto dist(int v, int u) const { return d(coords[v], coords[u]); };
};