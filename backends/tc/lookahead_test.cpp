#include <algorithm>
#include <fstream>
#include <memory>

#include "absl/flags/flag.h"
#include "backends/tc/instruction.h"
#include "backends/tc/ir_builder.h"
#include "backends/tc/p4c_interface.h"
#include "backends/tc/tcam_program.h"
#include "backends/tc/util.h"
#include "gtest/gtest.h"

namespace backends::tc {
namespace {

static constexpr char kLookaheadTestProgram[] =
    "backends/tc/testdata/lookahead.json";

class LookaheadTest : public testing::Test {
 protected:
  void SetUp() override {
    // Arguments to pass to the compiler.  p4c expects a char * array, rather
    // than a const char * array. So, we convert everything to string
    // implicitly, then take a reference to the first character. In the future
    // (when using C++17) we can switch to data() to create a second array of
    // pointers to data.
    std::string p4_program = kLookaheadTestProgram;

    std::vector<std::string> args{"p4c-tc", "--fromJSON", p4_program};
    std::vector<char *> argv;
    for (std::string &arg : args) {
      argv.push_back(&arg[0]);
    }

    // Set up P4C's infrastructure
    setup_gc_logging();
    setup_signals();

    AutoCompileContext compileContext(new P4TcContext);
    auto &options = P4TcContext::get().options();

    // Support P4-16 only
    options.langVersion = CompilerOptions::FrontendVersion::P4_16;

    // Parse compiler arguments
    options.process(argv.size(), argv.data());
    ASSERT_EQ(::errorCount(), 0);

    auto debug_hook = options.getDebugHook();

    // Load the IR
    std::ifstream json(options.file);
    ASSERT_TRUE(json) << "Could not open the JSON file containing P4 IR: "
                      << options.file;
    JSONLoader loader(json);
    const IR::Node *node = nullptr;
    loader >> node;
    const IR::P4Program *program = node->to<IR::P4Program>();
    ASSERT_TRUE(program) << options.file << "is not a P4 IR in JSON format.";

    // Run the P4C mid-end
    MidEnd mid_end(options);
    mid_end.addDebugHook(debug_hook);
    mid_end.process(program);

    ASSERT_EQ(::errorCount(), 0);

    backends::tc::IRBuilder ir_builder(mid_end.ref_map_, mid_end.type_map_);

    program->apply(ir_builder);
    ASSERT_FALSE(ir_builder.has_errors());

    delete program;

    tcam_program_ = ir_builder.tcam_program();
  }

  void TearDown() override {}

  absl::optional<TCAMProgram> tcam_program_;
};

// Helper function to find the last SetKey instruction in an entry
const SetKey *FindLastSetKeyInstruction(const TCAMEntry &tcam_entry) {
  const auto set_key = std::find_if(
      tcam_entry.instructions.rbegin(), tcam_entry.instructions.rend(),
      [](const std::shared_ptr<const Instruction> &instruction) {
        return bool(dynamic_cast<const SetKey *>(instruction.get()));
      });
  if (set_key == tcam_entry.instructions.rend()) {
    return nullptr;
  }
  return dynamic_cast<const SetKey *>(set_key->get());
}

TEST_F(LookaheadTest, LookaheadNoMove) {
  ASSERT_TRUE(tcam_program_)
      << "The IR builder has not generated the TCAM program";
  // Check if the set-key instruction generated for start state has the correct
  // offsets.
  const auto start_entry = tcam_program_->FindTCAMEntry("start", {}, {});
  ASSERT_NE(start_entry, nullptr)
      << "There is no entry for start, value=0, mask=0";
  const auto set_key = FindLastSetKeyInstruction(*start_entry);
  ASSERT_NE(set_key, nullptr) << "Could not find the set-key instruction";
  ASSERT_EQ(set_key->range(), util::Range::Create(0, 10).value())
      << "The range in the set-key instruction does not match the expected "
         "one.";
}

TEST_F(LookaheadTest, LookaheadMove) {
  ASSERT_TRUE(tcam_program_)
      << "The IR builder has not generated the TCAM program";
  // Check if the set-key instruction generated for start state has the correct
  // offsets.
  const auto entry = tcam_program_->FindTCAMEntry("state2_in", {}, {});
  ASSERT_NE(entry, nullptr)
      << "There is no entry for state2_in, value=0, mask=0";
  const auto set_key = FindLastSetKeyInstruction(*entry);
  ASSERT_NE(set_key, nullptr) << "Could not find the set-key instruction";
  const size_t begin = tcam_program_->HeaderSize("ethernet");
  ASSERT_EQ(set_key->range(), util::Range::Create(begin, begin + 16).value())
      << "The range in the set-key instruction does not match the expected "
         "one.";
}

}  // namespace
}  // namespace backends::tc
