#include <iostream>
#include <vector>
#include <random> // Используется для генерации массива
#include <stack> // Используется в сортировке
#include <iomanip>
#include "getcpu.c" // Используется в подсчете процессорного времени

/* Задание 1. C++ 14.
 *  Функция из условия:
 *  + Хорошая читаемость кода
 *  + Используется эффективный (в общем случае) алгоритм нахождения остатко от деления
 *  - Нельзя изменить бизнес-логику (например, (-2) % 2 вернет false. Иногда требуется проверить отрицательные
 *  числа на четность
 *  - Отдельная функция, при таком написании, является рудиментарной, т.к. возвращаемое значение может быть перенесено
 *  в место использования без потерия читаемости кода. Приведенная реализация занимает место в куче и тратит
 *  машинные циклы на вызов функции и возврат из нее.
 *  - Стандартая реализация взятия остатка использует алгоритм Евклида. Он эффективен в среднем, однако в этом
 *  конкретном случае малоэффективен (относительно моей реализации), т.к. использует цикл/рекурсию.
 */

/// Statement function
bool old_is_even(int _value) {
    return _value % 2;
}

/*  Аналог функции из условия (ради сохранения функциональности добавлено условие _value > 0):
 *  + Не использует циклы/рекурсию
 *  + Можно изменить бизнес-логику (например, сделать ноль и отрицательные числа четными)
 *  + Использует "дешевые" (относительно операций сложения и вычитания) битовые операции
 *  - Снижается читаемость кода
 *  - Невозможно перенести возвращаемое значение в место вызова без еще большей потери читаемости кода
 */

/// Statement function improvement
bool is_even(int _value) {
    return !(_value & 1) && _value > 0;
}

/* Задание 2.1 С++ 14
 * Шаблонный класс кольцевого буфера с принципом FIFO. Реализация опирается на массив элементов типа TSource.
 * + Работа добавления/извлечения за О(1)
 * + Проверка инварианта интуитивно понятна
 * - Сложности в определении указателей: один указывает на первый элемент, а другой - на первый "НЕ элемент"
 * - Довольно громоздкий (несмтря на свою простоту) код
 * - Занимает фиксированный объем памяти для хранения элементав
 */

/**
 * FIFO Circle buffer template class. Uses an array to store elements.
 *
 * @tparam TSource Type of stored elements
 *
 * @private
 * correct_collision();
 *
 * @public
 * push();
 * @public
 * pop();
 *
 * @example
 * CircleBuffer<int> a(10); // Makes circle buffer of ints sized 10.
 */
template<typename TSource>
class CircleBuffer {
private:
    // Дубликат определения типа для разделения указателей и размера
    typedef long long size_t;
    typedef long long pointer_t;
    typedef TSource element_t;

    /// Max. count of elements in buffer
    size_t capacity_ = 0;

    /// An array sized capacity
    element_t *storage_ = nullptr;

    /// first added buffer element
    pointer_t first_ = 0;
    /// first _not added_ element yet
    pointer_t last_ = 0;

    /**
     * A function to correct collision between buffer bounds, \b first_ and \b last_.
     *
     * @memberof CircleBuffer
     *
     * @category Core functions
     *
     * @param operation last operation flag: \b 0 - extract an element, \b 1 - addition
     *
     * @todo Basic logics was written on my lab, so this one may store some rudimental logics.
     */
    /* Следует следующей логике:
     * 1) last_ != first_
     * 2) last_ < capacity_ (иначе - last_ = 0)
     * 3) last_ >= 0 (иначе - last_ = capacity_ - 1)
     * Аналогично, относительно поля first_
    */
    void correct_collision(bool addition) {
        if (last_ >= capacity_) {
            last_ = 0;
            if (last_ == first_ && capacity_ > 1)
                ++first_;
        } else if (last_ < 0) {
            last_ = capacity_ - 1;
            if (last_ == first_ && capacity_ > 1)
                --first_;
        }

        if (first_ >= capacity_) {
            first_ = 0;
            if (first_ == last_ && capacity_ > 1)
                ++last_;
        } else if (first_ < 0) {
            first_ = capacity_ - 1;
            if (first_ == last_ && capacity_ > 1)
                --last_;
        }

        if (last_ == first_ && capacity_ > 1) {
            // Если последняя операция - добавление элемента в буфер, сместить указатель на первый элемент.
            // Иначе - сместить указатель на последний элемент.
            addition ? ++first_ : ++last_;
            // Проверка, остались ли поля в рамках массива. При этом не важно, какой из указателей был смещен.
            correct_collision(0);
        }
    }


public:
    CircleBuffer() noexcept {
        storage_ = nullptr;
    }

    explicit CircleBuffer(size_t size) {
        if (size < 0)
            throw std::runtime_error("Invalid size.");
        storage_ = new element_t[size];
        capacity_ = size;
    }

    CircleBuffer(CircleBuffer &source) noexcept {
        storage_ = source.storage_;
        capacity_ = source.size_;
        first_ = source.first_;
        last_ = source.last_;
    }

    /**
     * A method to push elements into circle buffer.
     *
     * @memberof CircleBuffer
     *
     * @category Public methods
     *
     * @param element An element to add
     *
     * @example
     * _.push(10); // Adds 10 into a circle buffer
     */

    void push(element_t element) noexcept {
        if (capacity_ == 0)
            return;
        storage_[last_++] = element;
        correct_collision(1);
    }

    /**
     * A method to extract first added element.
     *
     * @memberof CircleBuffer
     *
     * @category Public methods
     *
     * @return First added element at the circle buffer
     *
     * @example
     * _.pop()
     */
    element_t pop() noexcept {
        element_t needle = storage_[first_++];
        correct_collision(0);
        return needle;
    }

    /**
     * Operator to get the first added element without it's extract
     *
     * @param index An index of element needed. Must be equal to 0 (otherwise throws an exception).
     * @return First added element
     *
     * @example
     * a[10]; // Throws an exception
     * @example
     * a[0]; // Returns first added element
     */
    element_t &operator[](size_t index) const {
        if (index != 0) {
            throw std::runtime_error("Invalid index.\n");
        }
        return storage_[first_];
    }

    ~CircleBuffer() {
        delete[] storage_;
    }
};

/** Задание 2.2 С++ 14
 * FIFO Circle buffer template class. Uses structure to store key and next element pointer.
 *
 * @private
 * safe_push()
 *
 * @public
 * push()
 * @public
 * pop()
 *
 * @tparam TSource {Stored data type}
 *
 * @example
 * CircleBuffer<long long> a(10); // Makes circle buffer sized 10 with TSource = long long
 * @example
 * CircleBuffer<double> a(0); // Makes empty circle buffer
 * @example
 * CircleBuffer<int> a(1, 25); // // Makes circle buffer sized 1 with first element = 25
 */

/*
 * + Такая реализация проще понимается (лично мной)
 * + Гарантия инварианта обеспечивается буквально двумя строчками кода
 * + Занимает динамический объем памяти
 * - Вставка за О(N)
 * - Каждый элемент занимает больше памяти (для хранения указателя на следующий элемент)
 * - Необходима проверка на nullptr
 */
template<typename TSource>
class CircleBufferDynamic {
private:
    typedef long long size_t;
    typedef TSource element_t;

    /**
     * Local structure to store elements.
     *
     * @memberof CircleBufferDynamic
     *
     * @category Core structures
     *
     * @example
     * node a(b); // Generates node with fields \b key = b and \b next = nullptr
     */
    struct node {
        element_t key;
        node *next = nullptr;

        explicit node(element_t &_key) {
            key = _key;
        }
    };

    typedef node *pointer_t;

    pointer_t first_ = nullptr;
    pointer_t last_ = nullptr;

    size_t capacity_ = 0;
    size_t size_ = 0;

    /**
     * Метод добавления элементов в буфер (использует рекурсию).
     *
     * @memberof CircleBufferDynamic
     *
     * @category Private core functions
     *
     * @param current_node Current node (should be set equal to \b first_ when first time calling)
     * @param element An element to add
     */
    void safe_push(node *current_node, element_t &element) {
        if ((current_node->next == first_ || current_node->next == nullptr) && size_ < capacity_) {
            current_node->next = new node(element);
            last_ = current_node->next;
        } else if (size_ == capacity_) {
            last_->next->key = element;
            last_ = last_->next;
            first_ = last_->next;
        } else
            safe_push(current_node->next, element);
        last_->next = first_;
    }

public:
    explicit CircleBufferDynamic(size_t size) {
        if (size < 0)
            throw std::runtime_error("Invalid size.\n");
        capacity_ = size;
    }

    explicit CircleBufferDynamic(size_t size, element_t element) {
        if (size < 0)
            throw std::runtime_error("Invalid size.\n");
        capacity_ = size;
        first_ = new node(element);
        size_ = 1;
        last_ = first_;
    }

    /**
     * A safe_push() method decoratod.
     *
     * @memberof CircleBufferDynamic
     *
     * @category Public decorators
     *
     * @param element An element to add
     */
    void push(element_t element) {
        if (first_ == nullptr)
            first_ = new node(element);
        else
            safe_push(first_, element);
        size_ = size_ + 1 < capacity_ ? size_ + 1 : capacity_;
    }

    /**
     * A method to extract first element. If there's nothing to pop, throws an exception.
     *
     * @return needle First added element
     */
    element_t pop() {
        if (first_ == nullptr)
            throw std::runtime_error("Nothing to pop.\n");
        element_t needle = first_->key;
        first_ = first_->next;
        --size_;
        if (size_ == 0)
            first_ = nullptr;
        return needle;
    }
};

// Задание 3.
/* Рассуждения: процессорные тики (== процессорное время) == время исполнения. Следовательно, для неизвестной
 * выборки элементов массива, выгоднее всего выбрать QSort. Она выполняет порядка O(N^2) сравнений, однако не требует
 * дополнительной памяти + рекурсии для реализации (в отличие от Merge Sort). В среднем, QSort быстрее Merge Sort, поэтому
 * выбор пал на неё. И да, время работы можно улучшить, выбирая опорный элемент случайным образом (иначе возникает
 * худший случай времени работы за О(N^2). Подсчеты процессорного времени для конкретно этой реализации
 * и её тесты приведены ниже.
 */
/**
 * This is a helper class to allow access to global variables as local.
 *
 * @public
 * qsort()
 *
 * @tparam TSource Type of stored data
 */
template<typename TSource>
class Array {
private:
    std::vector<TSource> array_;
public:
    explicit Array(std::vector<TSource> &array) {
        array_ = array;
    }

    /**
     * Iterative realisation of QSort algorithm.
     *
     * @memberof Array
     * @category Sorting methods
     * @example
     * a.qsort(); // Sorts an Array a.
     */
    void qsort() {
        std::stack<long long> indexes; // Границы для рассмотрения
        indexes.push(0);
        indexes.push(array_.size() - 1);

        long long left; // Текущая левая граница массива
        long long right; // Текущая правая граница массива

        while (!indexes.empty()) {
            right = indexes.top();
            indexes.pop();
            left = indexes.top();
            indexes.pop();

            long long i = left;
            long long j = right;

            if ((right - left != 1) || (array_[left] <= array_[right])) {
                i = left;
                j = right;
                long long middle = array_[(left + right) / 2];
                // цикл продолжается, пока индексы i и j не сойдутся
                while (j > i) {
                    // пока i-ый элемент не превысит опорный
                    while ((middle > array_[i]))
                        ++i;
                    // пока j-ый элемент не окажется меньше опорного
                    while (array_[j] > middle)
                        --j;
                    if (i <= j)
                        std::swap(array_[i++], array_[j--]);
                }
            } else
                std::swap(array_[left], array_[right]);
            if (left < j) {
                indexes.push(left);
                indexes.push(j);
            }
            if (i < right) {
                indexes.push(i);
                indexes.push(right);
            }
        }
    }

    /// A helper method to store all generated arrays at one class instance
    void set(std::vector<TSource> &array) {
        array_ = array;
    }

    /// A method to get a sorted array
    std::vector<TSource> get() {
        return array_;
    }

    ~Array() {
        array_.clear();
    }
};

/*
 * Нерекурсивная реализация QSort. Измерения:
 * Time per array | Time per 100 arrays | Average CPU time
 * 0.0625000000s  | 5.0781250000s       | 0.0507812500s
 * 0.0625000000s  | 5.2656250000s       | 0.0527537129s
 * 0.0625000000s  | 5.0781250000s       | 0.0508972772s
 * 0.0468750000s  | 5.1875000000s       | 0.0518254950s
 * 0.0468750000s  | 5.1406250000s       | 0.0513613861s
 * 0.0625000000s  | 5.1250000000s       | 0.0513613861s
 * ________________________________________________________
 * Average CPU time per array: 0.0514967512s.
 */

int main() {
    CircleBufferDynamic<long long> qwerty(1);
    srand(rand() * UINT16_MAX + INT32_MIN);
    std::vector<long long> array(100000); // Массив из 1000000 случайных чисел
    for (int i = 0; i < array.size(); ++i) {
        array[i] = rand() * INT64_MAX + INT64_MIN;
    }

    Array<long long> sorting_array(array);


    double startTime, endTime; // Переменные для подсчета процессорного времени.
    // Используется алгоритм из статьи https://habr.com/ru/post/282301/ (2016 год)
    startTime = getCPUTime();
    sorting_array.qsort();
    endTime = getCPUTime();

    double res = endTime - startTime;

    std::cout << std::fixed << std::setprecision(10) << "QSorst CPU time usage: " << endTime - startTime << "s\n";

    startTime = getCPUTime();
    for (int i = 0; i < 100; ++i) {
        double offset = getCPUTime();
        srand(rand() * INT32_MAX + INT32_MIN);
        for (int i = 0; i < array.size(); ++i) {
            array[i] = rand() * INT64_MAX + INT64_MIN;
        }
        sorting_array.set(array);
        startTime += getCPUTime() - offset;
        sorting_array.qsort();
    }
    endTime = getCPUTime();

    std::cout << std::fixed << std::setprecision(10) << "QSorst CPU time usage after 100 iterations: "
              << endTime - startTime << "s\n";
    std::cout << std::fixed << std::setprecision(10) << "QSorst average CPU time usage: "
              << (endTime - startTime + res) / 101 << "s\n";

    return 0;
}