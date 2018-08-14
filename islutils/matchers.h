#include <isl/cpp.h>
#include <isl/schedule_node.h>
#include "islutils/scop.h"
#include <vector>

// A constraint is introduced by an access and a matcher.
// In more details, a constraint looks like (A, i0). Meaning that
// we have assigned dimension i0 to literal A.

namespace matchers {
  class RelationMatcher;
}

namespace constraints {

// represents single constraint.
typedef std::tuple<char, isl::pw_aff> singleConstraint;
// represents collection of constraints.
typedef std::vector<singleConstraint> MultipleConstraints;

// decouple matcher from constraint list.
struct ConstraintsList {
  int dimsInvolved = -1;
  MultipleConstraints constraints;
};

/*
ConstraintsList buildMatcherConstraintsWrites(
                matchers::RelationMatcher &matcher,
                isl::union_map &accessesWrites);

ConstraintsList buildMatcherConstraintsReads(
                matchers::RelationMatcher &matcher,
                isl::union_map &accessesReads);
*/
ConstraintsList buildMatcherConstraints(
		matchers::RelationMatcher &matcher,
		isl::union_map &accesses);

ConstraintsList compareLists(
                ConstraintsList &listOne,
                ConstraintsList &listTwo);


} // namespace constraint

/** \defgroup Matchers Matchers
 * \brief Structural matchers on schedule trees.
 *
 * A matcher is an object that captures the structure of schedule trees.
 * Conceptually, a matcher is a tree itself where every node is assigned a node
 * type.  The matcher class provides functionality to detect if a subtree in
 * the schedule tree has the same structure, that is the same types of nodes
 * and parent/child relationships.  Contrary to regular trees, matchers can be
 * constructed using nested call syntax omitting the details about the content
 * of nodes.  For example,
 *
 * ```
 * auto m = domain(
 *            context(
 *              sequence(
 *                filter(),
 *                filter())));
 * ```
 *
 * matches a subtree that starts at a domain node, having context as only
 * child, which in turn has a sequence as only child node, and the latter has
 * two filter children.  The structure is not anchored at any position in the
 * tree: the first node is not necessarily the tree root, and the innermost
 * node may have children of their own.
 */

/** \ingroup Matchers */
namespace matchers {

// It describes the type of matcher.
// read - read access
// write - write access
// readAndWrite - read and write access

enum class RelationKind { read, write, readAndWrite };

class RelationMatcher;

typedef std::vector<isl::pw_aff> matchingDims;

// TODO: extend to use variadic template
class RelationMatcher {
#define DECL_FRIEND_TYPE_MATCH(name)                                           \
  friend RelationMatcher name(char a, char b);                                 \
  friend RelationMatcher name(char a);
  DECL_FRIEND_TYPE_MATCH(read)
  DECL_FRIEND_TYPE_MATCH(write)
  DECL_FRIEND_TYPE_MATCH(readAndWrite)
#undef DECL_FRIEND_TYPE_MATCH

public:
  // is a read access?
  bool isRead() const;
  // is a write access?
  bool isWrite() const;
  // return literal at index i
  char getIndex(unsigned i) const;
  // get number of literals
  unsigned getIndexesSize() const;
  // set the dims of the matcher once known.
  void setDims(constraints::MultipleConstraints &mc);
  // get the type (read, write or readAndWrite)
  int getType() const;
  // get the isl::pw_aff for the dim. i
  std::vector<isl::pw_aff> getDims(int i) const;
  // get the accessed
  std::vector<isl::map> getAccesses(isl::union_map &accesses);
  // are the dimension set?
  bool isSet() const;
  // set flag for dimension(s)
  void set();
  ~RelationMatcher() = default;

private:
  // type (read, write or readAndWrite)
  RelationKind type_;
  // describe how the indexes should look like. Indexes layout.
  std::vector<char> indexes_;
  // once we figured out a combination that
  // satisfy all the matcher we "fixed" the
  // dimensions.
  std::vector<matchingDims> setDim_;
  bool isSetDim_;
};

class ScheduleNodeMatcher;

/** \defgroup MatchersStructuralCstr Structural Matcher Constructors.
 * \ingroup Matchers
 * These functions construct a structural matcher on the schedule tree by
 * specifying the type of the node (indicated by the function name).  They take
 * other matchers as arguments to describe the children of the node.  Depending
 * on the node type, functions take a single child matcher or an arbitrary
 * number thereof.  Sequence and set matcher builders take multiple children as
 * these types of node are the only ones that can have more than one child.
 * Additionally, all constructors are overloaded with an extra leading argument
 * to store a callback function for finer-grain matching.  This function is
 * called on the node before attempting to match its children.  It is passed
 * the node itself and returns true if the matching may continue and false if
 * it should fail immediately without processing the children.  When no child
 * matchers are provided, the node is allowed to have zero or more children.
 */
/** \{ */
template <typename Arg, typename... Args,
          typename = typename std::enable_if<
              std::is_same<typename std::remove_reference<Arg>::type,
                           ScheduleNodeMatcher>::value>::type>
ScheduleNodeMatcher sequence(Arg, Args... args);

template <typename Arg, typename... Args,
          typename = typename std::enable_if<
              std::is_same<typename std::remove_reference<Arg>::type,
                           ScheduleNodeMatcher>::value>::type>
ScheduleNodeMatcher sequence(isl::schedule_node &node, Arg, Args... args);

template <typename... Args>
ScheduleNodeMatcher sequence(std::function<bool(isl::schedule_node)> callback,
                             Args... args);

template <typename Arg, typename... Args,
          typename = typename std::enable_if<
              std::is_same<typename std::remove_reference<Arg>::type,
                           ScheduleNodeMatcher>::value>::type>
ScheduleNodeMatcher set(Arg, Args... args);

template <typename Arg, typename... Args,
          typename = typename std::enable_if<
              std::is_same<typename std::remove_reference<Arg>::type,
                           ScheduleNodeMatcher>::value>::type>
ScheduleNodeMatcher set(isl::schedule_node &node, Arg, Args... args);

template <typename... Args>
ScheduleNodeMatcher set(std::function<bool(isl::schedule_node)> callback,
                        Args... args);

ScheduleNodeMatcher band(isl::schedule_node &capture,
                         ScheduleNodeMatcher &&child);
ScheduleNodeMatcher band(ScheduleNodeMatcher &&child);
ScheduleNodeMatcher band(std::function<bool(isl::schedule_node)> callback,
                         ScheduleNodeMatcher &&child);

ScheduleNodeMatcher context(isl::schedule_node &capture,
                            ScheduleNodeMatcher &&child);
ScheduleNodeMatcher context(ScheduleNodeMatcher &&child);
ScheduleNodeMatcher context(std::function<bool(isl::schedule_node)> callback,
                            ScheduleNodeMatcher &&child);

ScheduleNodeMatcher domain(isl::schedule_node &capture,
                           ScheduleNodeMatcher &&child);
ScheduleNodeMatcher domain(ScheduleNodeMatcher &&child);
ScheduleNodeMatcher domain(std::function<bool(isl::schedule_node)> callback,
                           ScheduleNodeMatcher &&child);

ScheduleNodeMatcher extension(isl::schedule_node &capture,
                              ScheduleNodeMatcher &&child);
ScheduleNodeMatcher extension(ScheduleNodeMatcher &&child);
ScheduleNodeMatcher extension(std::function<bool(isl::schedule_node)> callback,
                              ScheduleNodeMatcher &&child);

ScheduleNodeMatcher filter(isl::schedule_node &capture,
                           ScheduleNodeMatcher &&child);
ScheduleNodeMatcher filter(ScheduleNodeMatcher &&child);
ScheduleNodeMatcher filter(std::function<bool(isl::schedule_node)> callback,
                           ScheduleNodeMatcher &&child);

ScheduleNodeMatcher guard(isl::schedule_node &capture,
                          ScheduleNodeMatcher &&child);
ScheduleNodeMatcher guard(ScheduleNodeMatcher &&child);
ScheduleNodeMatcher guard(std::function<bool(isl::schedule_node)> callback,
                          ScheduleNodeMatcher &&child);

ScheduleNodeMatcher mark(isl::schedule_node &capture,
                         ScheduleNodeMatcher &&child);
ScheduleNodeMatcher mark(ScheduleNodeMatcher &&child);
ScheduleNodeMatcher mark(std::function<bool(isl::schedule_node)> callback,
                         ScheduleNodeMatcher &&child);

ScheduleNodeMatcher leaf();

ScheduleNodeMatcher any(isl::schedule_node &capture);
ScheduleNodeMatcher any();
/** \} */

enum class ScheduleNodeType {
  Band,
  Context,
  Domain,
  Extension,
  Filter,
  Guard,
  Mark,
  Leaf,
  Sequence,
  Set,

  Any
};

inline isl_schedule_node_type toIslType(ScheduleNodeType type);
inline ScheduleNodeType fromIslType(isl_schedule_node_type type);

/** Node type matcher class for isl schedule trees.
 * \ingroup Matchers
 */
class ScheduleNodeMatcher {
#define DECL_FRIEND_TYPE_MATCH(name)                                           \
  template <typename... Args>                                                  \
  friend ScheduleNodeMatcher name(std::function<bool(isl::schedule_node)>,     \
                                  Args...);                                    \
  template <typename Arg, typename... Args, typename>                          \
  friend ScheduleNodeMatcher name(isl::schedule_node &, Arg, Args...);         \
  template <typename Arg, typename... Args, typename>                          \
  friend ScheduleNodeMatcher name(Arg, Args...);
  DECL_FRIEND_TYPE_MATCH(sequence)
  DECL_FRIEND_TYPE_MATCH(set)

#undef DECL_FRIEND_TYPE_MATCH

#define DECL_FRIEND_TYPE_MATCH(name)                                           \
  friend ScheduleNodeMatcher name(ScheduleNodeMatcher &&);                     \
  friend ScheduleNodeMatcher name(isl::schedule_node &,                        \
                                  ScheduleNodeMatcher &&);                     \
  friend ScheduleNodeMatcher name(std::function<bool(isl::schedule_node)>,     \
                                  ScheduleNodeMatcher &&);

  DECL_FRIEND_TYPE_MATCH(band)
  DECL_FRIEND_TYPE_MATCH(context)
  DECL_FRIEND_TYPE_MATCH(domain)
  DECL_FRIEND_TYPE_MATCH(extension)
  DECL_FRIEND_TYPE_MATCH(filter)
  DECL_FRIEND_TYPE_MATCH(guard)
  DECL_FRIEND_TYPE_MATCH(mark)

#undef DECL_FRIEND_TYPE_MATCH

  friend ScheduleNodeMatcher leaf();
  friend ScheduleNodeMatcher any();
  friend ScheduleNodeMatcher any(isl::schedule_node &);

private:
  explicit ScheduleNodeMatcher(isl::schedule_node &capture)
      : capture_(capture) {}

public:
  static bool isMatching(const ScheduleNodeMatcher &matcher,
                         isl::schedule_node node);

private:
  ScheduleNodeType current_;
  std::vector<ScheduleNodeMatcher> children_;
  std::function<bool(isl::schedule_node)> nodeCallback_;
  isl::schedule_node &capture_;
};

std::function<bool(isl::schedule_node)>
hasPreviousSibling(const ScheduleNodeMatcher &siblingMatcher);

std::function<bool(isl::schedule_node)>
hasNextSibling(const ScheduleNodeMatcher &siblingMatcher);

std::function<bool(isl::schedule_node)>
hasSibling(const ScheduleNodeMatcher &siblingMatcher);

std::function<bool(isl::schedule_node)>
hasDescendant(const ScheduleNodeMatcher &descendantMatcher);

class Finder {
  private:
    isl::union_map reads, writes;
    std::vector<RelationMatcher> readMatchers;
    std::vector<RelationMatcher> writeMatchers;
    std::vector<RelationMatcher> readAndWriteMatchers;
    void merge(std::vector<RelationMatcher> &first, 
	       std::vector<RelationMatcher> &second);
  public:
  Finder(isl::union_map reads,
	 isl::union_map writes, 
         std::vector<RelationMatcher> &matchers);
  int getSizeReadMatchers();
  int getSizeWriteMatchers();
  int getSizeReadAndWriteMatchers();
  void findAndPrint();
  ~Finder() = default;
};

} // namespace matchers

// debug functions.

//
//overloading << for printing std::vector<isl::set>
inline auto& operator<<(std::ostream &OS, std::vector<isl::set> &v) {
  for(size_t i=0; i<v.size(); ++i) {
    OS << v[i].to_str() << "\n";
  }
  return OS;
}

//
// overloading << for printing isl::space
inline auto& operator<<(std::ostream &OS, isl::space s) {
  OS << s.to_str() << "\n";
  return OS;
}

//
// helper function for printing single constraint.
inline void print_single_constraint(std::ostream &OS,
                                    const constraints::singleConstraint &c) {
  OS << std::get<0>(c) << "," << std::get<1>(c).to_str();
}

//
// overloading << for printing single constraint.
inline auto& operator<<(std::ostream &OS, const constraints::singleConstraint &c) {
  OS << "(";
  print_single_constraint(OS, c);
  return OS << ")";
}

//
// helper function for multiple constraints.
inline void print_multiple_constraints(std::ostream &OS,
                                       const constraints::MultipleConstraints &mc) {
  for(std::size_t i = 0; i < mc.size()-1; ++i) {
    OS << mc[i] << ",";
  }
  OS << mc[mc.size()-1];
}

//
// overloading << for multiple constraints.
inline auto& operator<<(std::ostream &OS, const constraints::MultipleConstraints &mc) {
  OS << "[";
  print_multiple_constraints(OS, mc);
  return OS << "]";
}

//
// overloading << for ConstraintsList
inline auto& operator<<(std::ostream &OS, const constraints::ConstraintsList &mc) {
  OS << "{";
  OS << "\n";
  OS << "Involved Dims = " << mc.dimsInvolved << "\n";
  if(mc.dimsInvolved == -1) {
    OS << "Constraints = empty";
    OS << "\n";
    return OS << "}";
  }
  OS << "Constraints = " << mc.constraints;
  OS << "\n";
  return OS << "}";
}

//
// overloading << for printing a std::vector<constraints::ConstrainsList>
inline auto& operator<<(std::ostream &OS, 
			const std::vector<constraints::ConstraintsList> &v) {
  for(size_t i=0; i<v.size(); ++i) {
    OS << v[i] << "\n";
  }
  return OS;
}

//
// overloading of << to print the entire structure of
// a matchers.
inline auto& operator<<(std::ostream &OS, const matchers::RelationMatcher &m) {

  OS << "@@@@@@\n";
  switch(m.getType()) {
  case 0:
    OS << "Read matcher\n";
    break;
  case 1:
    OS << "Write matcher\n";
    break;
  case 2:
    OS << "Read & Write matcher\n";
  default:
    OS << "NA\n";
  }

  int n_labels = m.getIndexesSize();
  for(int i=0; i<n_labels; ++i) {
    OS << m.getIndex(i) << "\n";
  }

  if(m.isSet()) {
    for(int i=0; i<n_labels; ++i){
      auto payload = m.getDims(i);
      for(size_t j=0; j<payload.size(); ++j) {
        OS << payload[j].to_str() << "\n";
      }
    }
  }
  return OS << "@@@@@@\n";
}

//
// overloading of << to print a std::vector<RelationMatcher>
inline auto& operator<<(std::ostream &OS,
                        const std::vector<matchers::RelationMatcher> &vm) {
  for(size_t i=0; i<vm.size(); ++i) {
    OS << vm[i] << "\n";
  }

  return OS;
}


//
// overloading of << to print std::vector<isl::map>
inline auto& operator<<(std::ostream &OS, const std::vector<isl::map> &v) {
  for(size_t i=0; i<v.size(); ++i) {
    OS << v[i].to_str() << "\n";
  }
  return OS << "\n";
}

//
// overloading of << to print isl::union_map 
inline auto& operator<<(std::ostream &OS, const isl::union_map &m) {
  return OS << m.to_str() << "\n";
}

//
// overloading of << to print std::vector<isl::pw_aff>
inline auto& operator<<(std::ostream &OS, const std::vector<isl::pw_aff> &v) {
  for(size_t i=0; i<v.size(); ++i) {
    OS << v[i].to_str() << "\n";
  }
  return OS;
}

//
// overloading of << to print isl::pw_aff
inline auto& operator<<(std::ostream &OS, const isl::pw_aff &a) {
  return OS << a.to_str() << "\n";
}

#include "matchers-inl.h"

