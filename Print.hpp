#ifndef Print_hpp
#define Print_hpp

namespace utils {
    // Print out the content of std::tuple
    // http://stackoverflow.com/questions/6245735/pretty-print-stdtuple
    namespace utils {
        template <std::size_t...> struct int_ {};

        template <std::size_t N, std::size_t... Is>
        struct gen_seq : gen_seq<N - 1, N - 1, Is...> {};

        template <std::size_t... Is> struct gen_seq<0, Is...> : int_<Is...> {};

        template <class Ch, class Tr, class Tuple, std::size_t... Is>
        void print_tuple(std::basic_ostream<Ch, Tr> &output, Tuple const &t,
                         int_<Is...>) {
            using swallow = int[];
            (void)swallow{
                0, (void(output << (Is == 0 ? "" : ", ") << std::get<Is>(t)),
                    0)...};
        }

        template <class Ch, class Tr, class... Args>
        auto operator<<(std::basic_ostream<Ch, Tr> &output,
                        std::tuple<Args...> const &t)
            -> std::basic_ostream<Ch, Tr> & {
            print_tuple(output, t, gen_seq<sizeof...(Args)>());
            return output;
        }

        template <class Ch, class Tr, typename First, typename Second>
        auto operator<<(std::basic_ostream<Ch, Tr> &output,
                        std::pair<First, Second> const &t)
            -> std::basic_ostream<Ch, Tr> & {
            output << "(" << t.first << "," << t.second << ")";
            return output;
        }
    }

    template <typename Data> void disp(Data &data, const std::string &message) {
        std::cout << message << "[ ";
        std::for_each(data.begin(), data.end(),
                      [](auto &val) { std::cout << val << " "; });
        std::cout << "]";
    }

    template <typename Data> void print(Data &data) {
        std::for_each(data.begin(), data.end(),
                      [](auto &val) { std::cout << val << "\n"; });
    }

    template <typename Container> void print(const Container &data) {
        for (auto const &val : data) {
            std::cout << val << "\n";
        }
    }
}

#endif
