# Домашняя работа №1 по дисциплине Алгоритмические Языки

## Портнов Пётр, группа ИУ8-25

### Задача

Разработать строковый калькулятор: пользователь вводит строку-выражение, программа
выводит на экран значение этого выражения.

### Техническое задание

Калькулятор должен уметь оперировать:

- знаками арифметических операций +, -, *, /, унарный – ;

- круглыми скобками ( ) ;

- тригонометрическими функциями sin, cos, tg, ctg;

- функциями exp, sqrt, возведение в степень;

- константами π и e;

- переменной x.


Программа должна:
1. быть написана с учетом всех известных вам особенностей ООП;
2. использовать значения констант из библиотеки cmath;
3. проверять корректность введённых выражений;
4. в случае некорректного выражения выводить сообщение пользователю;
5. в случае обнаружения в выражении переменной, запрашивать её значение у пользователя;
6. работать как с целочисленными значениями, так и с вещественными.

### Устройство программы

#### AST-модель

На основании введённого выражения пользователя программа строит **AST (Abstract Syntax Tree)** которая работает с абстрактным понятием (и соответствующим классом-интерфейсом) `Operation<T>`:

```c++
template<typename T>
class Operation {
public:
    virtual ~Operation() noexcept = default;

    /**
     * @brief Gets the result of this operation.
     *
     * @return result of this operation
     */
    virtual T result(Variables<T> const& variables) const = 0;
};
```

Данная абстракция описывает следующее поведение:

На вход подаётся некоторый набор именованных переменных в форме `Variables<T>`, каждая из которых имеет значение типа `T`, для которого возвращается результат выполнения данной операции над ними, имеющий также тип `T`.

Использование данного подхода позволяет абстрагироваться от конкретных операций, а также безболезненно использовать вложенные операции, то есть операции от операций.

Для удобной реализации данного подхода также использовались вспомогательные классы `UnaryOperation<T>` и `BinaryOperation<T>`, содержащие типичный код унарных и бинарных операций соответственно.

```c++
template<typename T>
class UnaryOperation : public Operation<T> {
    std::shared_ptr<Operation<T>> const operand_;

public:
    UnaryOperation(std::shared_ptr<Operation<T>> const operand) noexcept : operand_(operand) {}

    T result(Variables<T> const& variables) const final override { return apply(operand_->result(variables)); };

protected:
    virtual T apply(T&& value) const = 0;
};
```

```c++
template<typename T>
class BinaryOperation : public Operation<T> {
    std::shared_ptr<Operation<T>> const leftOperand_, rightOperand_;

public:
    BinaryOperation(std::shared_ptr<Operation<T>> const leftOperand,
                    std::shared_ptr<Operation<T>> const rightOperand) noexcept
        : leftOperand_(leftOperand), rightOperand_(rightOperand) {}

    T result(Variables<T> const& variables) const final override {
        return apply(leftOperand_->result(variables), rightOperand_->result(variables));
    };

protected:
    virtual T apply(T&& leftValue, T&& rightValue) const = 0;
};
```

Как видно, операнды хранятся как другие `Operation<T>` , обёрнутые в `shared_ptr`.

Также введена операция `ConstantOperation`, представляющая собой операцию, возвращающую фиксированное число:

```c++
template<typename T>
class ConstOperation final : public Operation<T> {
    T const result_;

public:
    explicit ConstOperation(T const& result) : result_(result) {}

    explicit ConstOperation(T&& result) noexcept : result_(std::move(result)) {}

    T result(Variables<T> const& variables) const override { return result_; }
};
```

Данная операция всегда является конечной в иерархии AST, поскольку не допускает дальнейшего углубления.

#### Парсер выражения

Для построения AST из исходного выражения, введённого пользователем, вводится понятие парсера выражения в форме `ExpressionParser<T>`, который для исходного выражения создаёт типизированную `Operation<T>`:

```c++
template<typename T>
class ExpressionParser {
public:
    virtual std::shared_ptr<Operation<T>> parse(std::istream& input) = 0;
};
```

#### Релизация парсера выражений

Задача написания парсера оказалась не столь тривиальной по ряду причин:

1. Наличие вложенных выражений
2. Различные приоритеты у операций
3. Различие в типе допустимого подвыражения, в зависимости от типа предыдущего
4. Коллизии названий основных функций и названий введённых пользователем переменных

Поэтому для реализации был использован усложнённый [алгоритм сортировочной станции]([https://ru.wikipedia.org/wiki/%D0%90%D0%BB%D0%B3%D0%BE%D1%80%D0%B8%D1%82%D0%BC_%D1%81%D0%BE%D1%80%D1%82%D0%B8%D1%80%D0%BE%D0%B2%D0%BE%D1%87%D0%BD%D0%BE%D0%B9_%D1%81%D1%82%D0%B0%D0%BD%D1%86%D0%B8%D0%B8](https://ru.wikipedia.org/wiki/Алгоритм_сортировочной_станции)) с предварительным разбиением выражения на слагаемые первого уровня:

1. Выражение минимизируется (удаляются все незначащие символы)
2. Сначала выражение разбивается над подвыражения-слагаемые с учётом знака, за счёт чего корнем AST будет операция множественного сложения (сложения из переменного числа параметров).
3. Далее все действия выполняются с каждым слагаемым независимо:
   1. Слагаемое посимвольно читается
   2. Определяются *токены* выражения - минимальные синтаксические единицы (числовые литералы, базовые математические операторы, обращения к переменным, вызовы функций, скобки)
   3. Токены загружаются в зависимости от текущего состояния системы в стек операторов с учётом приоритеты либо в очередь операций
   4. В итоговом состоянии, все данные со стека операторов отправляются в очередь операций
   5. Строится итоговое AST-представление данного слагаемого
4. Из всех слагаемых строится операция множественного сложения

Для удобства, в реализации были введены следующие вспомогательные сущности:

- Слагаемое:

  ```c++
  struct Term {
      std::string_view const expression;
      bool const negative;
  };
  ```

- Набор типов операторов:

  ```c++
  enum class OperatorType : uint8_t {
      // -- snip --
  }
  ```

- Оператор:

  ```c++
  struct Operator {
      OperatorType type;
      int priority;
      bool leftAssociative;
  };
  ```

- Контекст:

  ```c++
  struct Context final {
      typedef std::variant<OperationPointer, OperatorType> OperandOrOperator;
  
      std::queue<OperandOrOperator> output;
      std::stack<Operator> operators;
  
  public:
      void pushValue(std::shared_ptr<Operation<T>>&& value);
  
      void pushConstant(T&& number);
  
      void pushVariable(char const name);
  
      void pushFunction(OperatorType type);
  
      void pushOperator(Operator const& pushedOperator);
  
      void pushLeftBracket();
  
      void pushRightBracket();
  
      void flushRemaining();
  
      OperationPointer createOperation();
  };
  ```

- Набор разрешённых токенов (токены, которые может присутствовать в выражении в данный момент парсинга):

  ```c++
  enum class PermittedToken : uint8_t {
      OPERAND = 1 << 0u,
      ALGEBRAIC = 1 << 1u,
      MULTIPLYING = 1 << 2u,
  };
  ```

- Разрешения (средство управления набором разрешений):

  ```c++
  class Permissions final {
      uint8_t mask_;
  
  public:
      void clear();
  
      void permitAnything();
  
      void permit(PermittedToken token);
  
      inline bool isPermitted(PermittedToken token);
  
      void require(PermittedToken token);
  };
  ```

### Работа программы

В следующем сниппете приведён пример использования программы

```bash
$ ./algorithmic-languages-2-project-2
Enter the number of variables you would like to use (max: 10)
3
Enter the name of the variable and its value
x 15
Enter the name of the variable and its value
y 777
Enter the name of the variable and its value
z 12
Enter the expression to calculate
x^2 + (y * z + 3^2)
	=	9558
Enter `end`, `exit` or `stop` to end this session

Enter the number of variables you would like to use (max: 10)
0
Enter the expression to calculate
cos 0 + sin 0
	=	1
Enter `end`, `exit` or `stop` to end this session

Enter the number of variables you would like to use (max: 10)
1
Enter the name of the variable and its value
g 1024
Enter the expression to calculate
sqrt g
	=	32
Enter `end`, `exit` or `stop` to end this session

Enter the number of variables you would like to use (max: 10)
1
Enter the name of the variable and its value
x 2
Enter the expression to calculate
x + x * x
	=	6
Enter `end`, `exit` or `stop` to end this session
exit

Process finished with exit code 0
```

