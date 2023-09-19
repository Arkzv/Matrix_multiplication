#include <Python.h>
#include <pthread.h>
#include <stdlib.h>

typedef struct {
    double* A;
    double* B;
    double* C;
    int rows_A;
    int cols_A;
    int cols_B;
    int start_row;
    int end_row;
} MatrixMultiplyData;

void* matrix_multiply_range(void* data) {
    MatrixMultiplyData* matrix_data = (MatrixMultiplyData*)data;

    for (int i = matrix_data->start_row; i < matrix_data->end_row; ++i) {
        for (int j = 0; j < matrix_data->cols_B; ++j) {
            double sum = 0.0;
            for (int k = 0; k < matrix_data->cols_A; ++k) {
                sum += matrix_data->A[i * matrix_data->cols_A + k] * matrix_data->B[k * matrix_data->cols_B + j];
            }
            matrix_data->C[i * matrix_data->cols_B + j] = sum;
        }
    }

    pthread_exit(NULL);
}

static PyObject* multiply_matrices(PyObject* self, PyObject* args) {
    PyObject* py_A;
    PyObject* py_B;
    int num_threads;

    if (!PyArg_ParseTuple(args, "O!O!i", &PyList_Type, &py_A, &PyList_Type, &py_B, &num_threads)) {
        return NULL;
    }

    if (!PyList_Check(py_A) || !PyList_Check(py_B)) {
        PyErr_SetString(PyExc_TypeError, "Input matrices must be lists");
        return NULL;
    }

    int rows_A = PyList_Size(py_A);
    int cols_A = PyList_Size(PyList_GetItem(py_A, 0));
    int rows_B = PyList_Size(py_B);
    int cols_B = PyList_Size(PyList_GetItem(py_B, 0));

    if (cols_A != rows_B) {
        PyErr_SetString(PyExc_ValueError, "Matrix dimensions do not match for multiplication");
        return NULL;
    }

    double* A = (double*)malloc(rows_A * cols_A * sizeof(double));
    double* B = (double*)malloc(rows_B * cols_B * sizeof(double));
    double* C = (double*)malloc(rows_A * cols_B * sizeof(double));

    for (int i = 0; i < rows_A; ++i) {
        for (int j = 0; j < cols_A; ++j) {
            PyObject* elem_A = PyList_GetItem(PyList_GetItem(py_A, i), j);
            A[i * cols_A + j] = PyFloat_AsDouble(elem_A);
        }
    }

    for (int i = 0; i < rows_B; ++i) {
        for (int j = 0; j < cols_B; ++j) {
            PyObject* elem_B = PyList_GetItem(PyList_GetItem(py_B, i), j);
            B[i * cols_B + j] = PyFloat_AsDouble(elem_B);
        }
    }

    pthread_t* threads = (pthread_t*)malloc(num_threads * sizeof(pthread_t));
    MatrixMultiplyData* thread_data = (MatrixMultiplyData*)malloc(num_threads * sizeof(MatrixMultiplyData));

    int rows_per_thread = rows_A / num_threads;
    for (int i = 0; i < num_threads; ++i) {
        thread_data[i].A = A;
        thread_data[i].B = B;
        thread_data[i].C = C;
        thread_data[i].rows_A = rows_A;
        thread_data[i].cols_A = cols_A;
        thread_data[i].cols_B = cols_B;
        thread_data[i].start_row = i * rows_per_thread;
        thread_data[i].end_row = (i == num_threads - 1) ? rows_A : (i + 1) * rows_per_thread;
        pthread_create(&threads[i], NULL, matrix_multiply_range, (void*)&thread_data[i]);
    }

    for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], NULL);
    }

    free(threads);
    free(thread_data);
    free(A);
    free(B);

    PyObject* py_C = PyList_New(rows_A);
    for (int i = 0; i < rows_A; ++i) {
        PyObject* row = PyList_New(cols_B);
        for (int j = 0; j < cols_B; ++j) {
            PyObject* elem = PyFloat_FromDouble(C[i * cols_B + j]);
            PyList_SetItem(row, j, elem);
        }
        PyList_SetItem(py_C, i, row);
    }

    free(C);

    return py_C;
}

static PyMethodDef methods[] = {
    {"multiply_matrices", multiply_matrices, METH_VARARGS, "Multiply two matrices in parallel."},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef module = {
    PyModuleDef_HEAD_INIT,
    "matrix_multiplication",
    NULL,
    -1,
    methods
};

PyMODINIT_FUNC PyInit_matrix_multiplication(void) {
    return PyModule_Create(&module);
}
