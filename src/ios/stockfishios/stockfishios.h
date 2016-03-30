#include "threadbuf.h"
#import "stockfishcli.h"
#import "stockfish/uci.h"
#import "stockfish/bitboard.h"
#import "stockfish/position.h"
#import "stockfish/search.h"
#import "stockfish/evaluate.h"
#import "stockfish/pawns.h"
#import "stockfish/thread.h"
#import "stockfish/tt.h"
#import "stockfish/syzygy/tbprobe.h"
#import "../CordovaPluginStockfishc.h"

namespace stockfishios
{
  static std::string CMD_EXIT = "stockfish:exit";

  auto readstdout = [](void *stockfish) {
    std::streambuf* out = std::cout.rdbuf();

    threadbuf lichbuf;
    std::ostream lichout(&lichbuf);
    std::cout.rdbuf(lichout.rdbuf());
    std::istream lichin(&lichbuf);

    std::string o = "";

    while(o != CMD_EXIT) {
      std::string line;
      std::getline(lichin, line);
      if(line != CMD_EXIT) {
        const char* coutput = line.c_str();
        StockfishSendOutput(stockfish, coutput);
      } else {
        o = CMD_EXIT;
      }
    };

    // Restore output standard
    std::cout.rdbuf(out);

    lichbuf.close();
  };

  std::thread reader;

  void init(void *stockfish) {
    reader = std::thread(readstdout, stockfish);

    UCI::init(Options);
    PSQT::init();
    Bitboards::init();
    Position::init();
    Bitbases::init();
    Search::init();
    Eval::init();
    Pawns::init();
    Threads.init();
    Tablebases::init(Options["SyzygyPath"]);
    TT.resize(Options["Hash"]);
  }

  void cmd(std::string cmd) {
    stockfishcli::commandInit();
    stockfishcli::command(cmd);
  }

  void exit() {
    sync_cout << CMD_EXIT << sync_endl;
    reader.join();
    Threads.main()->join();
  }
}