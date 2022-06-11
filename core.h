#ifndef CORE_H
#define CORE_H
#include <iostream>
#include <functional>
#include <vector>
#include <cctype>
#include <string>
#include <stdexcept>

namespace Core {
	template<typename T>
	T Binpow(T a, int p) {
		T result = 1;
		while (p) {
			if (p & 1) result *= a;
			a *= a;
			p >>= 1;
		}
		return result;
	}

	template<typename T>
	class List {
	public:
		struct Node {
			T data;
			Node *next;
			Node *prev;
		};

	private:
		uint32_t size = 0;
		Node *head = nullptr, *tail = nullptr;

	public:
		List() = default;
		List(const List<T> & other) {
			Clear();
			Node* current = other.head;
			while (current) {
				InsertBack(current->data);
				current = current->next;
			}
		}
		~List() {
			Clear();
		}
		uint32_t Size() const {
			return size;
		}
		bool Empty() const {
			return !size;
		}
		Node* Head() const {
			return head;
		}
		Node* Tail() const {
			return tail;
		}
		List<T>& operator=(const List<T> & other) {
			Clear();
			Node* current = other.head;
			while (current) {
				InsertBack(current->data);
				current = current->next;
			}
			return *this;
		}
		void Clear() {
			Node* current = head, *next;
			while (current != nullptr) {
				next = current->next;
				delete current;
				current = next;
			}
			head = nullptr;
			tail = nullptr;
			size = 0;
		}
		Node* Find(const T & object) const {
			return Find([&object = static_cast<const T &>(object)](const T & cur_obj) -> bool {
				return object == cur_obj;
			});
		}
		Node* Find(std::function<bool(const T &)> comparator) const {
			Node *cur = head;
			while (cur) {
				if (comparator(cur->data)) break;
				cur = cur->next;
			}
			return cur;
		}
		Node* Get(uint32_t index) const {
			if (index >= size) return nullptr;
			Node *current = head;
			for (uint32_t i = 0; i < index; ++i)
				current = current->next;
			return current;
		}
		// I won't make another version with comparator function as it has to have overloaded <
		// Name may be a little misleading - it will find first thing that's >= than object
		// Since list is not nescessarily sorted it's not guaranteed to be the least of a kind
		Node* LowerBound(const T & object) const {
			Node *cur = head;
			while (cur) {
				if (!(cur->data < object)) break;
				cur = cur->next;
			}
			return cur;
		}
		void InsertAfter(Node *node, const T & data) {
			if (!node) return;
			Node* new_node = new Node;
			if (node == tail)
				tail = new_node;
			new_node->data = data;
			new_node->next = node->next;
			new_node->prev = node;
			node->next = new_node;
			if (new_node->next)
				new_node->next->prev = new_node;
			size++;
		}
		void InsertBefore(Node *node, const T & data) {
			if (!node) return;
			Node* new_node = new Node;
			if (node == head)
				head = new_node;
			new_node->data = data;
			new_node->next = node;
			new_node->prev = node->prev;
			node->prev = new_node;
			if (new_node->prev)
				new_node->prev->next = new_node;
			size++;
		}
		void InsertBack(const T & data) {
			if (Empty()) {
				head = new Node;
				head->data = data;
				head->next = head->prev = nullptr;
				tail = head;
				size = 1;
			} else {
				InsertAfter(tail, data);
			}
		}
		void InsertFront(const T & data) {
			if (Empty()) {
				head = new Node;
				head->data = data;
				head->next = head->prev = nullptr;
				tail = head;
				size = 1;
			} else {
				InsertBefore(head, data);
			}
		}
		void Delete(Node *node) {
			if (!node) return;
			if (node->prev)
				node->prev->next = node->next;
			if (node->next)
				node->next->prev = node->prev;
			if (node->next) {
				if (node->prev) {
					// deleting from somewhere in the middle, no need to do anything
				} else {
					// deleting head, should reassign it
					head = node->next;
				}
			} else {
				if (node->prev) {
					// deleting tail, should reassign it
					tail = node->prev;
				} else {
					// deleting the only element in the list
					head = tail = nullptr;
				}
			}
			delete node;
			size--;
		}
	};
	class Polynomial {
	public:
		enum ErrorType : uint8_t {
			OK = 0,
			UNKNOWN_CHARACTERS = 1, // 4(x^2)
			MULTIPLE_VARIABLES = 2, // x^2 + 4y^2
			EXPECTED_COEFFICIENT = 3, // x^2 +
			EXPECTED_VARIABLE = 4, // 4^3
			EXPECTED_POWER_SYMBOL = 5, // 4x3
			EXPECTED_DEGREE = 6 // 4x^x
		};
	private:
		struct Term {
			uint32_t degree;
			int32_t coeff;
			bool operator<(const Term & other) {
				return degree < other.degree;
			}
		};
		char var;
		// Stored as sorted
		List<Term> list;
		std::string string_view;
		bool updated;
		static constexpr uint32_t Q = 8;
		static constexpr uint32_t Q0 = 0;
		static constexpr uint32_t SIGMA = 5;
		static constexpr uint32_t FA[Q][SIGMA] = {
			{ 2, 3, Q, 1, 0 },
			{ 2, 3, Q, Q, 1 },
			{ 2, 3, Q, 1, 6 },
			{ Q, Q, 4, 1, 3 },
			{ 5, Q, Q, Q, 4 },
			{ 5, Q, Q, 1, 7 },
			{ Q, 3, Q, 1, 6 },
			{ Q, Q, Q, 1, 7 }
		};
		static constexpr ErrorType ERROR_ON_LEAVE[Q] = {
			OK,
			EXPECTED_COEFFICIENT,
			OK,
			OK,
			EXPECTED_DEGREE,
			OK,
			OK,
			OK
		};
		static constexpr ErrorType ERROR_ON_FAIL[Q] = {
			EXPECTED_COEFFICIENT,
			EXPECTED_COEFFICIENT,
			EXPECTED_VARIABLE,
			EXPECTED_POWER_SYMBOL,
			EXPECTED_DEGREE,
			EXPECTED_COEFFICIENT,
			EXPECTED_VARIABLE,
			EXPECTED_COEFFICIENT
		};
	private:
		std::pair<ErrorType, uint32_t> CheckForErrors(const std::string & str, char & varLetter) const {
			varLetter = '\0';
			for (uint32_t i = 0; i < str.size(); ++i) {
				if (std::isdigit(str[i]) || str[i] == '^' || str[i] == ' '
					|| str[i] == '+' || str[i] == '-') continue;
				if (!std::isalpha(str[i])) {
					return std::make_pair(UNKNOWN_CHARACTERS, i);
				}
			}
			uint32_t state = Q0;
			for (uint32_t i = 0; i < str.size(); ++i) {
				/* RIP Good Code. Frick QT Compiler. It rejected it...
				CHAR_TYPES.resize(256);
				std::fill(CHAR_TYPES.begin(), CHAR_TYPES.end(), 0);
				for (uint32_t i = (uint32_t) '0', till = (uint32_t) '9'; i <= till; ++i)
					CHAR_TYPES[i] = 0;
				for (uint32_t i = (uint32_t) 'A', till = (uint32_t) 'Z'; i <= till; ++i)
					CHAR_TYPES[i] = 1;
				for (uint32_t i = (uint32_t) 'a', till = (uint32_t) 'z'; i <= till; ++i)
					CHAR_TYPES[i] = 1;
				CHAR_TYPES[(uint32_t) '^'] = 2;
				CHAR_TYPES[(uint32_t) '+'] = CHAR_TYPES[(uint32_t) '-'] = 3;
				CHAR_TYPES[(uint32_t) ' '] = 4;
			*/
				uint32_t next = 0;
				if (std::isdigit(str[i])) next = 0;
				else if (std::isalpha(str[i])) next = 1;
				else if (str[i] == '^') next = 2;
				else if (str[i] == '+' || str[i] == '-') next = 3;
				else if (str[i] == ' ') next = 4;
				if (FA[state][next] == Q)
					return std::make_pair(ERROR_ON_FAIL[state], i);
				state = FA[state][next];
			}
			if (ERROR_ON_LEAVE[state] != OK)
				return std::make_pair(ERROR_ON_LEAVE[state], str.size());
			for (uint32_t i = 0; i < str.size(); ++i) {
				if (!std::isalpha(str[i])) continue;
				if (varLetter == '\0')
					varLetter = str[i];
				else if (varLetter != str[i])
					return std::make_pair(MULTIPLE_VARIABLES, i);
			}
			if (varLetter == '\0') varLetter = 'x';
			return std::make_pair(OK, 0);
		}
		std::string TermToString(const Term & term, bool first) const {
			std::string result;
			if (!first)
				result += ' ';
			if (!first && term.coeff > 0)
				result += "+ ";
			else if (term.coeff < 0)
				result += "- ";
			if (abs(term.coeff) != 1 || (abs(term.coeff) == 1 && term.degree == 0))
				result += std::to_string(abs(term.coeff));
			if (term.degree > 0)
				result += var;
			if (term.degree > 1)
				result += '^' + std::to_string(term.degree);
			return result;
		}
	public:
		Polynomial(): var('x'), updated(true) {}
		int GetCoefficient(uint32_t degree) {
			Term temp;
			temp.degree = degree;
			List<Term>::Node *ptr = list.LowerBound(temp);
			if (!ptr || ptr->data.degree != degree) return 0;
			else return ptr->data.coeff;
		}
		void AddTerm(const Term & term) {
			updated = true;
			if (list.Empty()) {
				list.InsertBack(term);
				return;
			}
			List<Term>::Node* place = list.LowerBound(term);
			if (!place) {
				list.InsertBack(term);
				return;
			}
			if (place->data.degree == term.degree) {
				if (place->data.coeff == -term.coeff)
					list.Delete(place);
				else
					place->data.coeff += term.coeff;
			} else {
				// degree is higher so we insert before
				list.InsertBefore(place, term);
			}
		}
		std::pair<ErrorType, uint32_t> InitFromString(const std::string & str) {
			char varLetter;
			std::pair<ErrorType, uint32_t> error = CheckForErrors(str, varLetter);
			if (error.first != OK)
				return error;
			var = varLetter;
			list.Clear();
			std::string str_upd; // no spaces
			for (char sym : str)
				if (sym != ' ')
					str_upd.push_back(sym);
			uint32_t i = 0;
			int32_t degree, coefficient;
			while (i < str_upd.size()) {
				bool positive = true;
				if (str_upd[i] == '+')
					++i;
				else if (str_upd[i] == '-')
					positive = false, ++i;
				degree = 0; coefficient = 0;
				if (std::isdigit(str_upd[i])) {
					for (; i < str_upd.size() && std::isdigit(str_upd[i]); ++i)
						coefficient = coefficient * 10 + ((int32_t) str_upd[i] - (int32_t) '0');
				} else {
					coefficient = 1;
				}
				if (!positive) coefficient = -coefficient;
				if (i < str_upd.size() && std::isalpha(str_upd[i])) {
					++i; // skipping variable letter
					if (i < str_upd.size() && str_upd[i] == '^') {
						++i; // skipping power symbol
						for (; i < str_upd.size() && std::isdigit(str_upd[i]); ++i)
							degree = degree * 10 + ((int32_t) str_upd[i] - (int32_t) '0');
					} else {
						degree = 1;
					}
				}
				Term new_term;
				new_term.degree = degree;
				new_term.coeff = coefficient;
				AddTerm(new_term);
			}
			updated = true;
			return std::make_pair(OK, 0);
		}
		std::string ExportAsString() {
			if (!updated)
				return string_view;
			if (list.Empty()) return std::string("0");
			std::string result;
			// As list is kept sorted in ascending order,
			// And we want polynomial with descending powers,
			// We have to go from back to front
			result = TermToString(list.Tail()->data, true);
			List<Term>::Node* current = list.Tail()->prev;
			while (current) {
				result += TermToString(current->data, false);
				current = current->prev;
			}
			updated = false;
			return string_view = result;
		}
		uint32_t Size() const {
			return list.Size();
		}
		bool Empty() const {
			return list.Empty();
		}
		friend void Add(const Polynomial &lhs, const Polynomial &rhs, Polynomial &res) {
			res.var = lhs.var;
			res.list.Clear();
			auto lp = lhs.list.Head(), rp = rhs.list.Head();
			while (lp && rp) {
				if (lp->data.degree == rp->data.degree) {
					res.list.InsertBack(lp->data);
					res.list.Tail()->data.coeff += rp->data.coeff;
					if (res.list.Tail()->data.coeff == 0)
						res.list.Delete(res.list.Tail());
					lp = lp->next;
					rp = rp->next;
				} else if (lp->data.degree < rp->data.degree) {
					res.list.InsertBack(lp->data);
					lp = lp->next;
				} else {
					res.list.InsertBack(rp->data);
					rp = rp->next;
				}
			}
			while (lp) {
				res.list.InsertBack(lp->data);
				lp = lp->next;
			}
			while (rp) {
				res.list.InsertBack(rp->data);
				rp = rp->next;
			}
		}
		friend void MultiplyByTerm(const Polynomial &lhs, const Term &term, Polynomial &res) {
			res.var = lhs.var;
			res.list.Clear();
			auto ptr = lhs.list.Head();
			while (ptr) {
				Term new_term;
				new_term.degree = ptr->data.degree + term.degree;
				new_term.coeff = ptr->data.coeff * term.coeff;
				res.list.InsertBack(new_term);
				ptr = ptr->next;
			}
		}
		friend void Multiply(const Polynomial &lhs, const Polynomial &rhs, Polynomial &res) {
			auto ptr = lhs.list.Head();
			while (ptr) {
				Polynomial temp_polynomial, temp_res;
				MultiplyByTerm(rhs, ptr->data, temp_polynomial);
				Add(res, temp_polynomial, temp_res);
				res = temp_res;
				ptr = ptr->next;
			}
		}
		friend void Derivative(const Polynomial & p, uint32_t n, Polynomial & res) {
			List<Term>::Node *current = p.list.Head();
			while (current) {
				if (current->data.degree >= n) {
					Term new_term = current->data;
					for (uint32_t i = 0; i < n; ++i) {
						new_term.coeff *= new_term.degree;
						--new_term.degree;
					}
					res.list.InsertBack(new_term);
				}
				current = current->next;
			}
		}
		template<typename T>
		T Evaluate(T x) const {
			List<Term>::Node *current = list.Head();
			T power = 1;
			int32_t degree = 0;
			T result = 0;
			while (current) {
				power *= Binpow(x, current->data.degree - degree);
				degree = current->data.degree;
				result += current->data.coeff * power;
				current = current->next;
			}
			return result;
		}
		std::vector<int32_t> GetRoots() const {
			if (list.Empty()) return std::vector<int32_t>();
			std::vector<int32_t> result;
			Term term = list.Head()->data;
			int32_t free_coefficient = abs(term.coeff);
			if (term.degree != 0)
				result.push_back(0);
			for (int32_t i = 1; i * i <= free_coefficient; ++i) {
				if (free_coefficient % i == 0) {
					if (Evaluate((int64_t) i) == 0) {
						result.push_back(i);
					}
					if (Evaluate((int64_t) -i) == 0) {
						result.push_back(-i);
					}
					if (Evaluate((int64_t) free_coefficient / (int64_t) i) == 0) {
						result.push_back(free_coefficient / i);
					}
					if (Evaluate((int64_t) -free_coefficient / (int64_t) i) == 0) {
						result.push_back(-free_coefficient / i);
					}
				}
			}
			sort(result.begin(), result.end());
			result.resize((uint32_t) (unique(result.begin(), result.end()) - result.begin()));
			return result;
		}
		Term& GetTerm(uint32_t index) {
			if (index >= list.Size())
				throw std::out_of_range("Polynomial with index " + std::to_string(index) + " isn't present in the base.");
			return list.Get(index)->data;
		}
	};
	class Base {
	private:
		List<Polynomial> list;
	public:
		Base() {}
		List<Polynomial> & GetList() {
			return list;
		}
		uint32_t Size() const {
			return list.Size();
		}
		bool Empty() const {
			return list.Empty();
		}
		List<Polynomial>::Node* Head() const {
			return list.Head();
		}
		List<Polynomial>::Node* Tail() const {
			return list.Tail();
		}
		std::pair<Polynomial::ErrorType, uint32_t> AddPolynomial(const std::string & str) {
			return AddPolynomial(str, list.Tail());
		}
		std::pair<Polynomial::ErrorType, uint32_t> AddPolynomial(const std::string & str, uint32_t index) {
			List<Polynomial>::Node *ptr = list.Get(index);
			return AddPolynomial(str, ptr);
		}
		std::pair<Polynomial::ErrorType, uint32_t> AddPolynomial(const std::string & str, List<Polynomial>::Node * node) {
			Polynomial new_polynomial;
			std::pair<Polynomial::ErrorType, uint32_t> error = new_polynomial.InitFromString(str);
			if (error.first != Polynomial::ErrorType::OK)
				return error;
			if (node)
				list.InsertAfter(node, new_polynomial);
			else
				list.InsertBack(new_polynomial);
			return std::make_pair(Polynomial::ErrorType::OK, 0);
		}
		void AddPolynomial(const Polynomial & p) {
			list.InsertBack(p);
		}
		void AddPolynomial(const Polynomial & p, uint32_t index) {
			List<Polynomial>::Node *ptr = list.Get(index);
			if (ptr)
				list.InsertAfter(ptr, p);
		}
		Polynomial& GetPolynomial(uint32_t index) const {
			if (index >= list.Size())
				throw std::out_of_range("Polynomial with index " + std::to_string(index) + " isn't present in the base.");
			List<Polynomial>::Node *node = list.Get(index);
			return node->data;
		}
		Polynomial& GetFirstPolynomial() const {
			if (list.Empty())
				throw std::out_of_range("No polynomials are present in the base.");
			return list.Head()->data;
		}
		Polynomial& GetLastPolynomial() const {
			if (list.Empty())
				throw std::out_of_range("No polynomials are present in the base.");
			return list.Tail()->data;
		}
		void DeletePolynomial(uint32_t index) {
			List<Polynomial>::Node *node = list.Get(index);
			list.Delete(node);
		}
		Polynomial AddPolynomials(uint32_t lhs_ind, uint32_t rhs_ind) const {
			Polynomial result;
			Polynomial& lhs = GetPolynomial(lhs_ind);
			Polynomial& rhs = GetPolynomial(rhs_ind);
			Add(lhs, rhs, result);
			return result;
		}
		Polynomial MultiplyPolynomials(uint32_t lhs_ind, uint32_t rhs_ind) const {
			Polynomial result;
			Polynomial& lhs = GetPolynomial(lhs_ind);
			Polynomial& rhs = GetPolynomial(rhs_ind);
			Multiply(lhs, rhs, result);
			return result;
		}
		Polynomial GetDerivative(uint32_t polynomial_ind, uint32_t n) const {
			Polynomial result;
			Polynomial& polynomial = GetPolynomial(polynomial_ind);
			Derivative(polynomial, n, result);
			return result;
		}
		std::vector<int> GetIntegerRoots(uint32_t polynomial_ind) const {
			Polynomial polynomial = GetPolynomial(polynomial_ind);
			return polynomial.GetRoots();
		}
	};
}
#endif // CORE_H
