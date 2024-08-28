#ifndef MATRIX_H
#define MATRIX_H

#include <vector>
#include <boost/serialization/vector.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <sstream>

struct Matrix {
    std::vector<std::vector<int>> data;

    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & data;
    }

    void display() const {
        for (const auto& row : data) {
            for (const auto& elem : row) {
                std::cout << elem << " ";
            }
            std::cout << std::endl;
        }
    }

};

#endif // MATRIX_H
