/**
 * implement a container like std::map
 */
#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

// only for std::less<T>
#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

namespace sjtu {

    template<
            class Key,
            class T,
            class Compare = std::less<Key>
    >
    class map {
    public:
        /**
         * the internal type of data.
         * it should have a default constructor, a copy constructor.
         * You can use sjtu::map as value_type by typedef.
         */
        typedef pair<const Key, T> value_type;
    private:
        struct node {
            node *f, *l, *r;
            value_type *key;
            int h;
            node(){key = nullptr, h = 1, f = nullptr, l = nullptr, r = nullptr;}

            node(const value_type &key_ , int h_ = 1, node *f_ = nullptr, node *l_ = nullptr, node *r_ = nullptr){
                key = new value_type(key_),h = h_,f = f_,l = l_,r =r_;
            }

            ~node() { delete key; }
        };

        node *root, *head, *nil;
        size_t num;
        Compare cmp;

        void flush_h(node *&x) {
            x->h = max(x->l->h, x->r->h) + 1;
        }

        node *find_key(const Key &key) const {
            node *x = root;
            while (x != nil) {
                if (cmp(key, x->key->first)) {
                    x = x->l;
                    continue;
                }
                if (cmp(x->key->first, key)) {
                    x = x->r;
                    continue;
                }
                break;
            }
            return x;
        }

        void map_copy(const map &other) {
            num = other.num;
            copy(root, nil, other.root, other.nil);
            head = get_min(root);
        }

        void copy(node *&x, node *f, node *o_x, node *o_nil) {
            if (o_x == o_nil) {
                x = nil;
                return;
            }
            x = new node(*new value_type(*o_x->key), o_x->h, f);
            copy(x->l, x, o_x->l, o_nil), copy(x->r, x, o_x->r, o_nil);
        }

        node *get_min(node *x) const {
            node *ret = x;
            while (x != nil) {
                ret = x, x = x->l;
            }
            return ret;
        }

        node *get_max(node *x) const {
            node *ret = x;
            while (x != nil) {
                ret = x, x = x->r;
            }
            return ret;
        }

        node *prv(node *x) const {
            if (x->l != nil) return get_max(x->l);
            for (; x->f != nil; x = x->f)
                if (x == x->f->r) return x->f;
            return nil;
        }

        node *nxt(node *x) const {
            if (x->r != nil) return get_min(x->r);
            for (; x->f != nil; x = x->f)
                if (x == x->f->l) return x->f;
            return nil;
        }


        void del(node *x) {
            if (x == nil) return;
            del(x->l);
            del(x->r);
            delete x;
        }

        void LL(node *&x) {
            node *f = x->f, *y = x->l, *z = y->r;
            x->l = z, z->f = x;
            y->r = x, x->f = y;
            y->f = f;
            flush_h(x), flush_h(y);
            x = y;
        }

        void RR(node *&x) {
            node *f = x->f, *y = x->r, *z = y->l;
            x->r = z, z->f = x;
            y->l = x, x->f = y;
            y->f = f;
            flush_h(x), flush_h(y);
            x = y;
        }

        void LR(node *&x) {
            RR(x->l);
            LL(x);
        }

        void RL(node *&x) {
            LL(x->r);
            RR(x);
        }

        void balance_insert(node *&x){

        }
        /**
         * see BidirectionalIterator at CppReference for help.
         *
         * if there is anything wrong throw invalid_iterator.
         *     like it = map.begin(); --it;
         *       or it = map.end(); ++end();
         */
    public:
        class const_iterator;

        class iterator {
            friend map;
        private:
            node *node_ptr;
            map *map_ptr;
        public:
            iterator() : node_ptr(nullptr), map_ptr(nullptr) {}

            iterator(node *node_ptr_, map *map_ptr_) : node_ptr(node_ptr_), map_ptr(map_ptr_) {}

            iterator(const iterator &other) : node_ptr(other.node_ptr), map_ptr(other.map_ptr) {}

            iterator operator++(int) {
                iterator ret = *this;
                ++*this;
                return ret;
            }

            iterator &operator++() {
                if (node_ptr == map_ptr->nil) throw invalid_iterator();
                node_ptr = map_ptr->nxt(node_ptr);
                return *this;
            }

            iterator operator--(int) {
                iterator ret = *this;
                --*this;
                return ret;
            }

            iterator &operator--() {
                if (node_ptr == map_ptr->head) throw invalid_iterator();
                if (node_ptr == map_ptr->nil)
                    node_ptr = map_ptr->get_max(map_ptr->root);
                else node_ptr = map_ptr->prv(node_ptr);
                return *this;
            }

            value_type &operator*() const {
                if (node_ptr == map_ptr->nil) throw invalid_iterator();
                return *node_ptr->key;
            }

            bool operator==(const iterator &rhs) const {
                return node_ptr == rhs.node_ptr;
            }

            bool operator==(const const_iterator &rhs) const {
                return node_ptr == rhs.node_ptr;
            }

            bool operator!=(const iterator &rhs) const {
                return !(*this == rhs);
            }

            bool operator!=(const const_iterator &rhs) const {
                return !(*this == rhs);
            }

            value_type *operator->() const noexcept {
                return &**this;
            }
        };

        class const_iterator {
            // it should has similar member method as iterator.
            //  and it should be able to construct from an iterator.
            friend map;
        private:
            // data members.
            node *node_ptr;
            const map *map_ptr;
        public:
            const_iterator() : node_ptr(nullptr), map_ptr(nullptr) {}

            const_iterator(node *node_ptr_,const map *map_ptr_) :node_ptr (node_ptr_), map_ptr(map_ptr_) {}

            const_iterator(const const_iterator &other) : node_ptr(other.node_ptr), map_ptr(other.map_ptr) {}

            const_iterator(const iterator &other) : node_ptr(other.node_ptr), map_ptr(other.map_ptr) {}

            const_iterator operator++(int) {
                const_iterator ret = *this;
                ++*this;
                return ret;
            }

            const_iterator &operator++() {
                if (node_ptr == map_ptr->nil) throw invalid_iterator();
                node_ptr = map_ptr->nxt(node_ptr);
                return *this;
            }

            const_iterator operator--(int) {
                const_iterator ret = *this;
                --*this;
                return ret;
            }

            const_iterator &operator--() {
                if (node_ptr == map_ptr->head) throw invalid_iterator();
                if (node_ptr == map_ptr->nil)
                    node_ptr = map_ptr->get_max(map_ptr->root);
                else node_ptr = map_ptr->prv(node_ptr);
                return *this;
            }

            const value_type &operator*() const {
                if (node_ptr == map_ptr->nil) throw invalid_iterator();
                return *node_ptr->key;
            }

            bool operator==(const iterator &rhs) const {
                return node_ptr == rhs.node_ptr;
            }

            bool operator==(const const_iterator &rhs) const {
                return node_ptr == rhs.node_ptr;
            }

            bool operator!=(const iterator &rhs) const {
                return !(*this == rhs);
            }

            bool operator!=(const const_iterator &rhs) const {
                return !(*this == rhs);
            }

            const value_type *operator->() const noexcept {
                return &**this;
            }
        };

        /**
         * TODO two constructors
         */
        map() {
            num = 0;
            nil = new node(), nil->h = 0;
            root = head = nil;
        }

        map(const map &other) {
            nil = new node();
            nil->h = 0;
            map_copy(other);
        }

        /**
         * TODO assignment operator
         */
        map &operator=(const map &other) {
            if (this == &other) return *this;
            del(root);
            map_copy(other);
            return *this;
        }

        /**
         * TODO Destructors
         */
        ~map() {
            del(root), delete nil;
        }

        /**
         * TODO
         * access specified element with bounds checking
         * Returns a reference to the mapped value of the element with key equivalent to key.
         * If no such element exists, an exception of type `index_out_of_bound'
         */
        T &at(const Key &key) {
            node *loc = find_key(key);
            if (loc == nil) throw index_out_of_bound();
            return loc->key->second;
        }

        const T &at(const Key &key) const {
            node *loc = find_key(key);
            if (loc == nil) throw index_out_of_bound();
            return loc->key->second;
        }

        /**
         * TODO
         * access specified element
         * Returns a reference to the value that is mapped to a key equivalent to key,
         *   performing an insertion if such key does not already exist.
         */
        T &operator[](const Key &key) {
            node *loc = find_key(key);
            if (loc == nil)
                return insert(value_type(key, T())).first.node_ptr->key->second;
            return loc->key->second;
        }

        /**
         * behave like at() throw index_out_of_bound if such key does not exist.
         */
        const T &operator[](const Key &key) const {
            return at(key);
        }

        /**
         * return a iterator to the beginning
         */
        iterator begin() {
            return iterator(head, this);
        }

        const_iterator cbegin() const {
            return const_iterator(head, this);
        }

        /**
         * return a iterator to the end
         * in fact, it returns past-the-end.
         */
        iterator end() {
            return iterator(nil, this);
        }

        const_iterator cend() const {
            return const_iterator(nil, this);
        }

        /**
         * checks whether the container is empty
         * return true if empty, otherwise false.
         */
        bool empty() const {
            return root == nil;
        }

        /**
         * returns the number of elements.
         */
        size_t size() const {
            return num;
        }

        /**
         * clears the contents
         */
        void clear() {
            num = 0;
            del(root);
            root = head = nil;
        }

        /**
         * insert an element.
         * return a pair, the first of the pair is
         *   the iterator to the new element (or the element that prevented the insertion),
         *   the second one is true if insert successfully, or false.
         */
        pair<iterator, bool> insert(const value_type &value) {
            node *pos = find_key(value.first);
            if (pos != nil) return pair<iterator, bool>(iterator(pos, this), false);
            if (empty()) {
                root = new node(value, 1, nil, nil, nil);
                head = get_min(root), ++num;
                return pair<iterator, bool>(iterator(root, this), true);
            }
            node *x = root, *f;
            while (1) {
                if(x == nil){
                    x = new node(value, 1,f,nil,nil);
                    break;
                }
                f = x;
                if (cmp(value.first, x->key->first))
                    x = x->l;
                else x = x->r;
            }
            balance_insert(x);
            head = get_min(root), ++num;
            return pair<iterator, bool>(iterator(x, this), true);
        }

        /**
         * erase the element at pos.
         *
         * throw if pos pointed to a bad element (pos == this->end() || pos points an element out of this)
         */
        void erase(iterator pos) {
            if (pos.map_ptr != this || pos.node_ptr == nil) throw invalid_iterator();
            node *x = pos.node_ptr;
        }

        /**
         * Returns the number of elements with key
         *   that compares equivalent to the specified argument,
         *   which is either 1 or 0
         *     since this container does not allow duplicates.
         * The default method of check the equivalence is !(a < b || b > a)
         */
        size_t count(const Key &key) const {
            return find_key(key) != nil;
        }

        /**
         * Finds an element with key equivalent to key.
         * key value of the element to search for.
         * Iterator to an element with key equivalent to key.
         *   If no such element is found, past-the-end (see end()) iterator is returned.
         */
        iterator find(const Key &key) {
            return iterator(find_key(key), this);
        }

        const_iterator find(const Key &key) const {
            return const_iterator(loc(key), this);
        }
    };

}

#endif