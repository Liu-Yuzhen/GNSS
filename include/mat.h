/*
* filename:mat.h
* abstract:override matrix * - + operator
* author:liuyuzhen
* date:2021.3.7
*/
#ifndef MAT_H
#define MAT_H


#include <iostream>
#include <assert.h>
#include <math.h>

namespace lyz{
enum Axis{
    xAxis = 0,
    yAxis = 1,
    zAxis = 2,
};

template<typename T>
class Point
{
public:
    Point(T x, T y, T z) :_x(x), _y(y), _z(z){}
    Point(){}

    template<typename U>
    friend std::ostream& operator<<(std::ostream& ost, const Point<U>& pt) {
        ost << "[" << pt._x << ", " << pt._y << ", " << pt._z << "]";
        return ost;
    }

    Point<T> operator+(const Point<T>& pt) { return Point<T>(_x + pt._x, _y + pt._y, _z + pt._z); }
    Point<T> operator-(const Point<T>& pt) { return Point<T>(_x - pt._x, _y - pt._y, _z - pt._z); }
    Point<T> operator*(T scaler) { return Point<T>(_x * scaler, _y * scaler, _z * scaler); }
    friend Point<T> operator *(T scalar, const Point<T>& pt){
        return Point<T>(scalar*pt._x, scalar*pt._y, scalar*pt._z);
    }
    T norm() { return sqrt(_x * _x + _y * _y + _z * _z); }
    T _x, _y, _z;
};

template<typename T>
class Mat{
public:
    Mat(size_t row, size_t col):_rows(row), _cols(col){_array = new T[row * col];}
    Mat(T* array, size_t row, size_t col);
    Mat(const Mat<T>&);
    Mat() {}
    ~Mat();


    static Mat<T> creat(double theta, Axis axis);
    static Mat<T> zeros(size_t rows, size_t cols);
    static Mat<T> identity(size_t rows);

    inline T& at(size_t i, size_t j)const;


    inline size_t cols()const{ return _cols; }
    inline size_t rows()const{ return _rows; }
    inline size_t total()const{ return _cols * _rows; }
    inline bool empty(){return _array == nullptr;}
    inline T* ptr()const{ return _array; }

    template<typename U>
    friend std::ostream& operator<<(std::ostream&, const Mat<U>&);

    
    const Mat<T> operator *(const Mat<T>&)const;
    const Mat<T> operator +(const Mat<T>&)const;
    const Mat<T> operator -(const Mat<T>&)const;
    const Point<T> operator *(const Point<T>&)const;
    const Mat<T> operator *(T)const;
    const Mat<T>& operator =(const Mat<T>&);//deep copy
private:
    T* _array = nullptr;
    size_t _cols = 0, _rows = 0;
};



template<typename T>
Mat<T>::Mat(T* array, size_t row, size_t col):_rows(row), _cols(col){
    _array = new T[row*col];
    memcpy(_array, array, row*col*sizeof(T));
}

template<typename T>
Mat<T>::Mat(const Mat<T>& rhs){
    _rows = rhs.rows();
    _cols = rhs.cols();
    _array = new T[_rows*_cols];
    memcpy(_array, rhs.ptr(), _rows*_cols*sizeof(T));
}

template<typename T>
Mat<T>::~Mat(){
    delete[] _array;
}

template<typename T>
const Mat<T>& Mat<T>::operator =(const Mat<T>& rhs){
    if (this == &rhs){
        return *this;
    }
    
    _rows = rhs.rows();
    _cols = rhs.cols();
    _array = new T[_rows*_cols];
    memcpy(_array, rhs.ptr(), _rows*_cols*sizeof(T));

    return *this;
}

template<typename T>
const Mat<T> Mat<T>::operator +(const Mat<T>& rhs)const{
    assert(rhs.cols() == _cols && rhs.rows() == _rows);

    T* val = new T[_rows * _cols];
    const T* arrA = rhs.ptr();

    for (size_t m = 0; m < _rows; m++){
        size_t start = m * _cols;
        for (size_t n = 0; n < _cols; n++){
            val[start + n] = arrA[start + n] + _array[start + n];
        }
    }

    return Mat<T>(val, _rows, _cols);
}

template<typename T>
const Mat<T> Mat<T>::operator -(const Mat<T>& rhs)const{
    assert(rhs.cols() == _cols && rhs.rows() == _rows);

    T* val = new T[_rows * _cols];
    const T* arrA = rhs.ptr();

    for (size_t m = 0; m < _rows; m++){
        size_t start = m * _cols;
        for (size_t n = 0; n < _cols; n++){
            val[start + n] = _array[start + n] - arrA[start + n];
        }
    }

    return Mat<T>(val, _rows, _cols);
}

template<typename T>
const Mat<T> Mat<T>::operator *(const Mat<T>& rhs)const{
    assert(_cols == rhs.rows());
    
    T* val = new T[_rows * rhs.cols()];
    T* arrA = rhs.ptr();

    for (size_t m = 0; m < _rows; m++){
        for (size_t n = 0; n < rhs.cols(); n++){
            T sum = 0;
            for (size_t k = 0; k < _cols; k++){
                sum += _array[m * _cols + k] * arrA[k * _rows + n];
            }
            val[m * _cols + n] = sum;
        }
        
    }

    return Mat<T>(val, rhs.rows(), _cols);
}

template<typename T>
const Mat<T> Mat<T>::operator *(T lambda)const{
    T* arr = new T[_rows * _cols];
    for (size_t i = 0; i < _rows; i++){
        size_t start = i * _cols;

        for (size_t j = 0; j < _cols;j++){
            arr[start + j] = lambda * _array[start + j];
        }
    }

    return Mat<T>(arr, _rows, _cols);
}


template<typename T>
const Point<T> Mat<T>::operator *(const Point<T>& pt)const {
    assert((_cols == 4 && _rows == 4));
    Point<T> res;
    res._x = at(0, 0) * pt._x + at(0, 1) * pt._y + at(0, 2) * pt._z + at(0, 3);
    res._y = at(1, 0) * pt._x + at(1, 1) * pt._y + at(1, 2) * pt._z + at(1, 3);
    res._z = at(2, 0) * pt._x + at(2, 1) * pt._y + at(2, 2) * pt._z + at(2, 3);
    return res;
}

template<typename U>
std::ostream& operator<<(std::ostream& ost, const Mat<U>& rhs){
    U* arr = rhs.ptr();
    size_t r = rhs.rows(), c = rhs.cols();
    for (size_t m = 0; m < r; m++){
        size_t start = m * c;
        for (size_t n = 0; n < c; n++){
            ost << arr[start + n] << "\t";
        }
        ost << std::endl;
    }
    return ost;
}

template<typename T>
T& Mat<T>::at(size_t i, size_t j)const{
    assert(i < _rows && j < _cols);
    return *(&_array[i * _cols + j]);
}

template<typename T>
Mat<T> Mat<T>::zeros(size_t rows, size_t cols){
    T* arr = new T[rows * cols];
    for (size_t i = 0; i < rows * cols; i++) {
        arr[i] = 0;
    }
    return Mat<T>(arr, rows, cols);
}

template<typename T>
Mat<T> Mat<T>::creat(double theta, Axis axis){
    Mat<T> mat = Mat<T>::zeros(4, 4);
    mat.at(3, 3) = 1;
    switch (axis)
    {
    case xAxis:
        /* code */
        mat.at(0, 0) = 1;
        mat.at(1, 1) = cos(theta);
        mat.at(1, 2) = sin(theta);
        mat.at(2, 1) = -sin(theta);
        mat.at(2, 2) = cos(theta);
        break;
    case yAxis:
        mat.at(1, 1) = 1;
        mat.at(0, 0) = cos(theta);
        mat.at(0, 2) = -sin(theta);
        mat.at(2, 0) = sin(theta);
        mat.at(2, 2) = cos(theta);
        break;

    case zAxis:
        mat.at(2, 2) = 1;
        mat.at(0, 0) = cos(theta);
        mat.at(0, 1) = sin(theta);
        mat.at(1, 0) = -sin(theta);
        mat.at(1, 1) = cos(theta);
        break;
    default:
        break;
    }

    return mat;
}

template<typename T>
Mat<T> Mat<T>::identity(size_t rows){
    T* arr = new T(rows * rows);
    for ( int i = 0; i < rows; i++){
        arr[rows * i + rows] = 1;
    }
    return Mat<T>(arr, rows, rows);
}



typedef Mat<double> Matd;
typedef Mat<float> Matf;
typedef Point<double> Pointd;
typedef Point<float> Pointf;
#define PI 3.141592653589793238462643383279502884197169399

}


#endif
