##  (2025-06-08)

### Features

* added 50-move ply counter to Position ([911bf8d](https://github.com/benthevining/libchess/commit/911bf8d30e4428585f54338a7b32fcc009e171dd))
* added 50-move ply counter to Position ([#3](https://github.com/benthevining/libchess/issues/3)) ([41da016](https://github.com/benthevining/libchess/commit/41da016a8763b2837c2e808a1b3f0f62c27c8a8c))
* added castling rights to Position class ([#3](https://github.com/benthevining/libchess/issues/3)) ([5eee208](https://github.com/benthevining/libchess/commit/5eee20805c6668349a4226bee24bcfd55126c427))
* all string parsing functions now handle string trimming defensively ([#44](https://github.com/benthevining/libchess/issues/44)) ([77555fd](https://github.com/benthevining/libchess/commit/77555fddf54494e1f869db37793ecbc984167ca8))
* any_legal_moves() function ([582ae26](https://github.com/benthevining/libchess/commit/582ae267b47c2cdc83b8d8efd4d3dbf83e6271b8))
* basic move ordering ([#96](https://github.com/benthevining/libchess/issues/96)) ([aadcd14](https://github.com/benthevining/libchess/commit/aadcd142a10ad46da0baa114f67a7bba3c53d5f2))
* basic time control algorithm ([#94](https://github.com/benthevining/libchess/issues/94)) ([41af4ec](https://github.com/benthevining/libchess/commit/41af4ec3c6e20d1b08eba9c90653ba8fc7005ddb))
* bitboard fill algorithms ([a243b64](https://github.com/benthevining/libchess/commit/a243b64aa492d85b8628a729877f9eb32a41e2b9))
* Bitboard first() and last() functions ([bfe4d04](https://github.com/benthevining/libchess/commit/bfe4d044b595850aa7c7c597845878ace9f53fe6))
* bitboard flipping functions ([55aaac7](https://github.com/benthevining/libchess/commit/55aaac788d6b333badbfba71c0526e3c6409d78c))
* bitboard masks for all files ([bb2d68f](https://github.com/benthevining/libchess/commit/bb2d68f21d72300bd0bf4c4153818dd4132132f0))
* bitboard masks for all ranks ([055b0c8](https://github.com/benthevining/libchess/commit/055b0c8815d6f1e0ce14104281e598bc7d86a898))
* bitboard masks for center, perimeter ([dce8e2c](https://github.com/benthevining/libchess/commit/dce8e2c8425a3a59c236818ff6c0cb09a15b4e7e))
* bitboard masks for piece starting positions ([348a972](https://github.com/benthevining/libchess/commit/348a972a9b114bf82bde0a424acb55ee3eca31ee))
* bitboard masks for ranks, files, diagonals ([f535c95](https://github.com/benthevining/libchess/commit/f535c95aef94ff7ba4f74557170d0ccd5082e9ee))
* Bitboard shift functions ([#51](https://github.com/benthevining/libchess/issues/51)) ([384f7a3](https://github.com/benthevining/libchess/commit/384f7a33f043022c169b2c9e981d6272010f4803))
* Bitboard::indices() method ([2b2dd46](https://github.com/benthevining/libchess/commit/2b2dd46734618bee3ae901317b03056688a90bde))
* calculating bishop ray attacks ([c66d6d7](https://github.com/benthevining/libchess/commit/c66d6d76217d56289a954bbf1c0e0e7dde1bad99))
* color enumeration ([#11](https://github.com/benthevining/libchess/issues/11)) ([cf165cd](https://github.com/benthevining/libchess/commit/cf165cdffd17cd075ad3e857577bf7d431313ac1))
* detecting draw by insufficient material ([#75](https://github.com/benthevining/libchess/issues/75)) ([ac16ee1](https://github.com/benthevining/libchess/commit/ac16ee14c59ff95cbb2f7bb9d6b083c730dab517))
* detecting threefold repetitions ([#67](https://github.com/benthevining/libchess/issues/67)) ([bcdc557](https://github.com/benthevining/libchess/commit/bcdc55756ba82e0b6370064407dd07d6d0aebfd0))
* **eval:** Bishop pair bonus ([a71cd0f](https://github.com/benthevining/libchess/commit/a71cd0fffa44a07f8e55c0b3072d94af72f334c7))
* function to calculate possible bishop moves ([28c81ee](https://github.com/benthevining/libchess/commit/28c81eed18e99be8875e4109d4f3790b4d001fd8))
* function to calculate possible queen moves ([c4a9b99](https://github.com/benthevining/libchess/commit/c4a9b997c7aa53a0e4b0955bd7ba0ec1ace18a6f))
* function to calculate possible rook moves ([044a7aa](https://github.com/benthevining/libchess/commit/044a7aa7f836c154e006916ff0427d90c5146550))
* function to generate captures only ([#89](https://github.com/benthevining/libchess/issues/89)) ([cac9cf5](https://github.com/benthevining/libchess/commit/cac9cf5ca7e4acc8f3d6df38c64ba1d83a35479d))
* function to generate possible king moves ([6f3882a](https://github.com/benthevining/libchess/commit/6f3882af8f794b25d7401aa030e79c0e9a2b3887))
* function to generate possible knight moves ([#8](https://github.com/benthevining/libchess/issues/8)) ([7591151](https://github.com/benthevining/libchess/commit/75911511e223becbc809112f899d075c582146df))
* function to get attacked squares, Position::is_check() ([66a09cf](https://github.com/benthevining/libchess/commit/66a09cfbb1a7c7b47cd3a8b865a9c074c7cd5c66))
* function to parse Position from FEN ([#18](https://github.com/benthevining/libchess/issues/18)) ([4c68076](https://github.com/benthevining/libchess/commit/4c680764219780204f768a80933b6026f2f887f6))
* function to print Position as UTF8 ([bc7caf3](https://github.com/benthevining/libchess/commit/bc7caf30caee3e8f52d5e3688607ed169fda79e5))
* function to print Position as UTF8 ([#3](https://github.com/benthevining/libchess/issues/3)) ([85d43de](https://github.com/benthevining/libchess/commit/85d43de704e889e6a8fadf1b2b9652dcd350ad21))
* function to write Position as FEN ([#18](https://github.com/benthevining/libchess/issues/18)) ([b748f8e](https://github.com/benthevining/libchess/commit/b748f8e5d74dba3e9e9ab5fd7c190125f07c2601))
* functions for pawn front- and rear-fills ([#46](https://github.com/benthevining/libchess/issues/46)) ([e819512](https://github.com/benthevining/libchess/commit/e8195122a46124d86895dd1f18c354e48a4280a1))
* functions to calculate pawn pushes & attacks ([08f4394](https://github.com/benthevining/libchess/commit/08f4394c26f5f72eb8d8e0df97a1bf01f309e117))
* GameRecord::get_final_position() ([8a0f9d5](https://github.com/benthevining/libchess/commit/8a0f9d5a25336a541392c395fc8b3bf83d0721b0))
* generate_legal_moves() function ([f42a746](https://github.com/benthevining/libchess/commit/f42a74624e0ad8c56e1e246d913550025ef90739))
* generating en passant captures setwise ([#54](https://github.com/benthevining/libchess/issues/54)) ([053ebaf](https://github.com/benthevining/libchess/commit/053ebafeba9db08565543346332f5ddb3df9a51d))
* generating pawn captures setwise ([#54](https://github.com/benthevining/libchess/issues/54)) ([80ff127](https://github.com/benthevining/libchess/commit/80ff1272753557699424cfc83d6934c2478fe897))
* handling comments in PGN ([69e733d](https://github.com/benthevining/libchess/commit/69e733d2d7081d999c336ef3132cdefd3582a6b4))
* handling NAGs in PGN ([73470c5](https://github.com/benthevining/libchess/commit/73470c582117cdbc3344bd3b2b60e67b7a544622))
* initial commit of bitboard class ([0dd7917](https://github.com/benthevining/libchess/commit/0dd791763d18c98be4e444a5d584bdaea96ed756))
* initial commit of bitboard class ([#1](https://github.com/benthevining/libchess/issues/1)) ([bfad3bc](https://github.com/benthevining/libchess/commit/bfad3bca8d1478b9b402754d8b3cb8d5ef5b2f39))
* initial commit of BitboardSet class ([#3](https://github.com/benthevining/libchess/issues/3)) ([816efd0](https://github.com/benthevining/libchess/commit/816efd042719071baa45c2ecbeea72e7c8b2ac0e))
* initial commit of EPD handling functions ([2ee773d](https://github.com/benthevining/libchess/commit/2ee773dcd5d27a81a6e7f9adb7bca7da4c6b5ea6))
* initial commit of evaluation header ([#23](https://github.com/benthevining/libchess/issues/23)) ([6592294](https://github.com/benthevining/libchess/commit/6592294c8884978894786476f067de1e52135e08))
* initial commit of FEN variation parsing ([95c8be5](https://github.com/benthevining/libchess/commit/95c8be5b6ee504175722a31e29ac081f0ac086b4))
* initial commit of magic bitboard function skeletons ([46527ed](https://github.com/benthevining/libchess/commit/46527edc087baaa148b3b7b49be0fd66f5a6d261))
* initial commit of magic bitboard functions ([cffa750](https://github.com/benthevining/libchess/commit/cffa750c3e457d0241fedaf4e2af5f1f2933f742))
* initial commit of Move class ([#6](https://github.com/benthevining/libchess/issues/6)) ([57ae7c0](https://github.com/benthevining/libchess/commit/57ae7c07e85eabd2c7bf25331da0714aba36c0e4))
* initial commit of negamax search function ([9a3ad56](https://github.com/benthevining/libchess/commit/9a3ad564766f178230a2ee8df0af9526444e22a1))
* initial commit of PGN parsing ([#95](https://github.com/benthevining/libchess/issues/95)) ([e6cd812](https://github.com/benthevining/libchess/commit/e6cd8124bfcf564faeceb4ccd2cefffa510a0508))
* initial commit of piece square tables ([c603e10](https://github.com/benthevining/libchess/commit/c603e10e12366ed543e25005533ac3c60012a57d))
* initial commit of piece-square table values ([e2700c3](https://github.com/benthevining/libchess/commit/e2700c3f751dcc86a6ce0b68eefac1902b654004))
* initial commit of Position class ([#3](https://github.com/benthevining/libchess/issues/3)) ([0602c66](https://github.com/benthevining/libchess/commit/0602c6681d18a10ca0d49303eea2f22e78b90197))
* initial commit of Square class ([#2](https://github.com/benthevining/libchess/issues/2)) ([0390bd2](https://github.com/benthevining/libchess/commit/0390bd2e58e90b7f95b21e69ce8dbd80638a193f))
* initial commit of UCI default options ([#84](https://github.com/benthevining/libchess/issues/84)) ([a1be297](https://github.com/benthevining/libchess/commit/a1be297fc831f2cbe391be9b10884ece4b8892a2))
* initial commit of UCI move serialization ([#43](https://github.com/benthevining/libchess/issues/43)) ([02723fb](https://github.com/benthevining/libchess/commit/02723fbcc2f92c27506c918629f2177e7fe50455))
* initial commit of Zobrist hashing ([#33](https://github.com/benthevining/libchess/issues/33)) ([ee06800](https://github.com/benthevining/libchess/commit/ee0680002bfb5e650d58f99ec3f1d1cfb62fe501))
* initial implementation of basic search ([#72](https://github.com/benthevining/libchess/issues/72)) ([950abc0](https://github.com/benthevining/libchess/commit/950abc0131de2687b06ce647c52138cb0a226007))
* initial implementation of UCI protocol ([#71](https://github.com/benthevining/libchess/issues/71)) ([517259b](https://github.com/benthevining/libchess/commit/517259b6a73f07995283bef54d6b37dd7063a246))
* knight_distance(), chebyshev_distance() functions ([#10](https://github.com/benthevining/libchess/issues/10)) ([b3b0422](https://github.com/benthevining/libchess/commit/b3b042289423aaecd37ed350cdc81df82c308062))
* legal move generation functions ([a17fd09](https://github.com/benthevining/libchess/commit/a17fd0982edec34dc424b6e0f8bb158913fd958d))
* masks::files::get(File) and masks::ranks::get(Rank) ([81c6b24](https://github.com/benthevining/libchess/commit/81c6b2491352f4c2f85f6c26973273ca3b7014e4))
* more fill algorithms ([a766ec5](https://github.com/benthevining/libchess/commit/a766ec570372f120d69956efc5c91c092d8c509c))
* more fill algorithms ([#52](https://github.com/benthevining/libchess/issues/52)) ([dc870ef](https://github.com/benthevining/libchess/commit/dc870efa47a088f8f94e989f69b4dd2a1b35c54f))
* move creation functions for castling ([#6](https://github.com/benthevining/libchess/issues/6)) ([d3f37a1](https://github.com/benthevining/libchess/commit/d3f37a1182ad1cde5fdc93586ba2fe801929c20a))
* Move promotion() creation function ([#6](https://github.com/benthevining/libchess/issues/6)) ([915208f](https://github.com/benthevining/libchess/commit/915208f19c4491923256947cba88d044f4edbc92))
* Move::from_string() ([3728f2b](https://github.com/benthevining/libchess/commit/3728f2b35748b8bd9937b641dbc8876395c1dad4))
* moves::generate_for() ([dba1529](https://github.com/benthevining/libchess/commit/dba152978d4e2fe9dcd9be702446d2cc6a599a61))
* optimized Bitboard::indices() using a custom iterator ([8c7c18c](https://github.com/benthevining/libchess/commit/8c7c18ccc66f4a0b45fe1a1e30468134f50fbd23))
* parse_all_pgns() function ([#103](https://github.com/benthevining/libchess/issues/103)) ([457f661](https://github.com/benthevining/libchess/commit/457f661b82e7d293fa81f98058279f7d5db432a9))
* patterns::bishop() is now done set-wise ([6bd4abe](https://github.com/benthevining/libchess/commit/6bd4abe6947b2a9f28ad56db0722fddedb3b38d3))
* patterns::queen() is now done set-wise ([ab5e5cf](https://github.com/benthevining/libchess/commit/ab5e5cf6b3b99043ef1321a6eb3678463c40d5cf))
* PGN writing ([#95](https://github.com/benthevining/libchess/issues/95)) ([d48be35](https://github.com/benthevining/libchess/commit/d48be35b5c6466316446a842084861ded57976eb))
* **PGN:** parsing of variations ([ccd33e6](https://github.com/benthevining/libchess/commit/ccd33e6fb9b074a174defb1e33733e0d986db595))
* **PGN:** writing of variations ([ae30b0a](https://github.com/benthevining/libchess/commit/ae30b0a6dbfb7fc7c9304bbc9dc3c0b983ebf559))
* piece type enumeration ([#7](https://github.com/benthevining/libchess/issues/7)) ([2918312](https://github.com/benthevining/libchess/commit/29183121defad049545851e98785c44637a13032))
* piece type from string function ([#12](https://github.com/benthevining/libchess/issues/12)) ([07036b8](https://github.com/benthevining/libchess/commit/07036b8d6b94e85e3b5d95bbd99e4e9171144d19))
* Pieces::get_piece_on(Square) ([f9fe28a](https://github.com/benthevining/libchess/commit/f9fe28aee3cff1f5cc7591470a82a40bb2b780d8))
* Pieces::has_bishop_pair() ([620bb1c](https://github.com/benthevining/libchess/commit/620bb1c2705d8676bc2b96d2af3e1915be4be3c4))
* Pieces::is_file_half_open() ([2198dae](https://github.com/benthevining/libchess/commit/2198dae2b7bc618eb1795013306a9d226b7a7666))
* Position is_checkmate(), is_stalemate() functions ([#28](https://github.com/benthevining/libchess/issues/28)) ([43eef7e](https://github.com/benthevining/libchess/commit/43eef7e5b45b373d53ff9e004abb63069e05f1fe))
* Position is_file_half_open()/get_half_open_files() ([#3](https://github.com/benthevining/libchess/issues/3)) ([0d2afb4](https://github.com/benthevining/libchess/commit/0d2afb4143557322a3712d7bd3d8bd2b1385f78c))
* Position::is_capture(Move) ([bcb146d](https://github.com/benthevining/libchess/commit/bcb146db533340655a8d2209ec53f4e5ff9b6dcc))
* Position::is_draw() ([#24](https://github.com/benthevining/libchess/issues/24)) ([e14f280](https://github.com/benthevining/libchess/commit/e14f2808f2f57dd65e389eae597b5b1394acca6a))
* Position::is_fifty_move_draw() ([#24](https://github.com/benthevining/libchess/issues/24)) ([cdbd445](https://github.com/benthevining/libchess/commit/cdbd4454cf06e005e5ac9b5d44151c58eab7d00f))
* Position::make_move() ([#6](https://github.com/benthevining/libchess/issues/6)) ([faeaee9](https://github.com/benthevining/libchess/commit/faeaee9090a27102fe5f95f88e56657dfc9207e9))
* Position::move_from_string() function ([#21](https://github.com/benthevining/libchess/issues/21)) ([3ff7549](https://github.com/benthevining/libchess/commit/3ff754984275fab3bd7f67c0f115e2ace5790bbc))
* prev_pawn_rank(), next_pawn_rank() functions ([#49](https://github.com/benthevining/libchess/issues/49)) ([12106cf](https://github.com/benthevining/libchess/commit/12106cf23ed1e857ac9d221546943dc434d2979e))
* set-wise sliding piece attack generation ([#50](https://github.com/benthevining/libchess/issues/50)) ([f1d924e](https://github.com/benthevining/libchess/commit/f1d924e5615a28b73e4ada39010d922df17e94ea))
* skeleton of UCI program ([#71](https://github.com/benthevining/libchess/issues/71)) ([127038e](https://github.com/benthevining/libchess/commit/127038ed427ba3257a6fe6f0caba88d04528f78d))
* some utility functions for distances between squares ([03bd790](https://github.com/benthevining/libchess/commit/03bd790c9a112554cc79b551f56739731b5a875e))
* Square to/from string ([#2](https://github.com/benthevining/libchess/issues/2)) ([20cd15c](https://github.com/benthevining/libchess/commit/20cd15ce89ce21c62f598c20febe04de6ad2b1cd))
* std::formatter for Bitboard class ([0fa510e](https://github.com/benthevining/libchess/commit/0fa510ebc5235310e9c2edf96f275bdba27ac4ee))
* std::formatter for Move class ([#6](https://github.com/benthevining/libchess/issues/6)) ([a51393d](https://github.com/benthevining/libchess/commit/a51393d85357d5cc4d64873c46deb14e2dfe6932))
* std::formatter for piece type enum ([#12](https://github.com/benthevining/libchess/issues/12)) ([6329f8d](https://github.com/benthevining/libchess/commit/6329f8dfa58bfba0114c61729b36fbaa5aa87fc0))
* UCI engine base class ([#88](https://github.com/benthevining/libchess/issues/88)) ([7e2f6cc](https://github.com/benthevining/libchess/commit/7e2f6cc607646c63beab6e8cf68ef8f293cc689b))
* UCI option classes ([#82](https://github.com/benthevining/libchess/issues/82)) ([45f85ee](https://github.com/benthevining/libchess/commit/45f85ee76da0f10a98de0f11d3508b22688b08f6))

### Bug Fixes

* added Position::is_en_passant() method ([79972bb](https://github.com/benthevining/libchess/commit/79972bb2f48f5304d2d839048795860d02c96cbf))
* algebraic notation ([703a7a2](https://github.com/benthevining/libchess/commit/703a7a2daaf774343949533ff70674d0bf9c2a07))
* algebraic notation of pawn captures ([71faef6](https://github.com/benthevining/libchess/commit/71faef69a503a70b4367f2b65f51cd48ceb0ace2))
* algebraic notation starting square disambiguation ([4512937](https://github.com/benthevining/libchess/commit/451293767b86032ec5c87ff3b6da649d20531067))
* bitboard vertical flipping ([02c9e17](https://github.com/benthevining/libchess/commit/02c9e17ced68cce5b2b3b172aa1358614758467e))
* bug in FEN output ([358ed2f](https://github.com/benthevining/libchess/commit/358ed2f160f56386e85dc8da2b7c5647f31b30fd))
* castling logic ([c40f1b6](https://github.com/benthevining/libchess/commit/c40f1b6b337babe8c964562202fc409fd2799e63))
* castling logic ([89b8f09](https://github.com/benthevining/libchess/commit/89b8f095ca77b3beec945dca359ee5397cbaf159))
* cmake ([8f0dcc0](https://github.com/benthevining/libchess/commit/8f0dcc00eb54fd17af44d73cce0d33f37d6754d9))
* correctly removing captured pawn from bitboard when en passant capture is made ([#60](https://github.com/benthevining/libchess/issues/60)) ([f8e0765](https://github.com/benthevining/libchess/commit/f8e07658be6bb6e2e205e2c239637c51e525932f))
* disallowed castling out of check ([#59](https://github.com/benthevining/libchess/issues/59)) ([ecb91e6](https://github.com/benthevining/libchess/commit/ecb91e68537f0c055ec4a74db4886b902872b6d7))
* en passant captures ([2d220a1](https://github.com/benthevining/libchess/commit/2d220a1b0c917dc9a1e48ba8581595216458534c))
* FEN parsing ([115fb41](https://github.com/benthevining/libchess/commit/115fb412fc8c2f239a736a96da636e36119e00e9))
* MSVC fix ([512b89d](https://github.com/benthevining/libchess/commit/512b89db68af4fcd9de99ef2b683b2938b80eaf2))
* MSVC fix ([6b0fe31](https://github.com/benthevining/libchess/commit/6b0fe31d937bfe88e578c0f58cc02931d9a396eb))
* MSVC fixes ([bfbf530](https://github.com/benthevining/libchess/commit/bfbf530f11e836063fcb9593a55f07f0a0534b75))
* optimizing ray attack generation using fill algorithms ([7fb745a](https://github.com/benthevining/libchess/commit/7fb745a0269f5a5bfba7ed048301dfbe269f970e))
* perft stats discrepancies ([#68](https://github.com/benthevining/libchess/issues/68)) ([7962fa5](https://github.com/benthevining/libchess/commit/7962fa5d51d67a0c0f8aec2e9b82519abbc6e801))
* PGN parsing ([#95](https://github.com/benthevining/libchess/issues/95)) ([46dca28](https://github.com/benthevining/libchess/commit/46dca28daaf9a137f1a6e7fd968003f6abe7603c))
* **PGN:** handling custom starting position FEN ([a7422df](https://github.com/benthevining/libchess/commit/a7422dfe461350b88ed4d94f6c2de1f3a9cf7578))
* **PGN:** no longer expecting '*' at end of movelist for an ongoing game ([95887ac](https://github.com/benthevining/libchess/commit/95887ac770cb8e60bdde708d68605bf57b18a8de))
* **PGN:** parsing & printing of nested variations ([b7bc5d9](https://github.com/benthevining/libchess/commit/b7bc5d94b1565dc1e335113b4d207509ed186879))
* **PGN:** tolerating spaces between move number & move ([5d091f8](https://github.com/benthevining/libchess/commit/5d091f837fa91df4f556519f203279d540f7aabf))
* **PGN:** tolerating spaces between move number & move ([3f8f172](https://github.com/benthevining/libchess/commit/3f8f172e405a75451ce3170aa0792dc5ec884cfe))
* **PGN:** writing Setup metadata tag if custom startng position is used ([2700aed](https://github.com/benthevining/libchess/commit/2700aed9ac53ea7aff6fd746674aad73546249c9))
* Position::is_check() ([#29](https://github.com/benthevining/libchess/issues/29)) ([473576d](https://github.com/benthevining/libchess/commit/473576da5b5004a4d480c4edfc3aac3ee629904f))
* PST color flipping ([888be9b](https://github.com/benthevining/libchess/commit/888be9bc9a11202cbddd264a6d7bf1460fbb07dc))
* removing castling rights when a rook is captured ([c17cbd1](https://github.com/benthevining/libchess/commit/c17cbd1b3556a2209a2672424b9ed04e05280d9e))
* search algorithm ([34534e8](https://github.com/benthevining/libchess/commit/34534e80519a393613cd31dc817245fe6d450877))
* search algorithm ([3622500](https://github.com/benthevining/libchess/commit/3622500ec7f94f9b796b1011e9d4d8329a5d49f3))
* **search:** alpha/beta limits are now greater than mate score ([81d97a6](https://github.com/benthevining/libchess/commit/81d97a626aa89ea65f662d94ecad2339bc45aa4a))
* **search:** scoring mate based on ply count from root ([4c0dd47](https://github.com/benthevining/libchess/commit/4c0dd47df65eb084e9c8b0518e21d312ed04ae4d))
* threefold repetitions ([#67](https://github.com/benthevining/libchess/issues/67)) ([1e68fd7](https://github.com/benthevining/libchess/commit/1e68fd79a8324f3fe99a35b9c7d01b7cd76f8238))
* **uci:** using std::getline() ([c6cc584](https://github.com/benthevining/libchess/commit/c6cc5840a7f5919ef52eed9832714fa374175dff))
* Windows fixes ([cd51a5c](https://github.com/benthevining/libchess/commit/cd51a5c3ad6b2ead4fcc160fc71434d0a4851248))
* writing "seven tag roster" in required order ([47bb00a](https://github.com/benthevining/libchess/commit/47bb00a5ec404338e29ceee3ba5f718280dfcbbd))
* writing game result to created PGN strings ([#95](https://github.com/benthevining/libchess/issues/95)) ([2a52d92](https://github.com/benthevining/libchess/commit/2a52d929585e41dd2e5e48a1a63fd0ab16008c13))
## [1.1.0](https://github.com/benthevining/libchess/compare/v1.0.0...v1.1.0) (2025-06-13)

### Features

* basic replacement scheme for transposition table ([1308c4e](https://github.com/benthevining/libchess/commit/1308c4e36c0c9981fbe91e16e2f7809c5ba1e8ff))
* clear() method for transposition table ([656a669](https://github.com/benthevining/libchess/commit/656a669100652c114eda408f2e7f54b956b7ac40))
* function to get PV from transposition table ([#123](https://github.com/benthevining/libchess/issues/123)) ([0c5c453](https://github.com/benthevining/libchess/commit/0c5c45372430deeea2103ec84db542ff3cd60a3c))
* initial commit of searcher thread class ([f207b89](https://github.com/benthevining/libchess/commit/f207b89f5fc633976a4ef2b7f79bc94bf55274fd))
* initial commit of transposition table ([e78c42e](https://github.com/benthevining/libchess/commit/e78c42ed3c70a9497542e41669a9a1d37cd3c974))
* search function can now restrict search to specific moves ([#130](https://github.com/benthevining/libchess/issues/130)) ([bcf19b3](https://github.com/benthevining/libchess/commit/bcf19b30fbd10c1875d199d9fb2a7e7c6131b4bc))
* search function now has option to limit number of searched nodes ([#130](https://github.com/benthevining/libchess/issues/130)) ([1efa158](https://github.com/benthevining/libchess/commit/1efa158adb743a5adb8bf4a38b140ec5937264fd))
* search is now interruptible using an atomic flag ([45969e6](https://github.com/benthevining/libchess/commit/45969e6cb41c3f2c18783102c0b6ccef15a060bc))

### Bug Fixes

* iterative deepening ([503ffbd](https://github.com/benthevining/libchess/commit/503ffbd10d1372a222c7f6174b6f7c786c7739e5))
* MSVC fixes ([99fdf7e](https://github.com/benthevining/libchess/commit/99fdf7e9b926718598447fde8015ba5f8a14387f))
* reverted clang-tidy fix ([925fe27](https://github.com/benthevining/libchess/commit/925fe27014e4753cb7b42a4c004070539f224bdf))
* using transposition table in quiescence search function ([7783073](https://github.com/benthevining/libchess/commit/77830730871bb0637add24163fc5c80b47bc7e50))
## [1.2.0](https://github.com/benthevining/BenBot/compare/v1.1.0...v1.2.0) (2025-07-12)

### Features

* **ben-bot:** help command now prints ASCII logo ([#167](https://github.com/benthevining/BenBot/issues/167)) ([3a0930a](https://github.com/benthevining/BenBot/commit/3a0930a457188888759677cfa33869f330e43609))
* **book:** using cmrc to embed book data ([a451eef](https://github.com/benthevining/BenBot/commit/a451eefe7a5fd5efeff9c89f53e7ecc350e3f931))
* button to clear transposition table ([#162](https://github.com/benthevining/BenBot/issues/162)) ([1ae8092](https://github.com/benthevining/BenBot/commit/1ae80922108c08d5f9de60aacabbe6de9aa50302))
* **eval:** basic space evaluation ([2f30147](https://github.com/benthevining/BenBot/commit/2f301478e9377375385e6251b54bf47c3244507b))
* **eval:** bonus for connected rooks ([5ad0266](https://github.com/benthevining/BenBot/commit/5ad02667361840a9282061ed55b045b0c08524de))
* **eval:** bonus for connected rooks ([#119](https://github.com/benthevining/BenBot/issues/119)) ([c43329f](https://github.com/benthevining/BenBot/commit/c43329f534394e56acb7956d81757b9f25833c63))
* **eval:** bonus for controlling center squares ([b15de66](https://github.com/benthevining/BenBot/commit/b15de66c570c19da16839f5205db00a0f4e227ae))
* **eval:** bonus for having last non-pawn piece ([eca3811](https://github.com/benthevining/BenBot/commit/eca3811af5d5d43e48f259a4b8ca3d2921e92846))
* **eval:** bonus for passed pawns ([#40](https://github.com/benthevining/BenBot/issues/40)) ([3415d63](https://github.com/benthevining/BenBot/commit/3415d6348587d890db8fe5dab58a24d5705c925e))
* **eval:** bonus for pawn chains ([#121](https://github.com/benthevining/BenBot/issues/121)) ([0b73a17](https://github.com/benthevining/BenBot/commit/0b73a179320f0dd80b62d24e818c56ce2bbd1137))
* **eval:** bonus for rooks on open files ([#119](https://github.com/benthevining/BenBot/issues/119)) ([9a6e894](https://github.com/benthevining/BenBot/commit/9a6e8943b496a466d775cc9052e0909d26249323))
* **eval:** endgame "mop up" score ([#120](https://github.com/benthevining/BenBot/issues/120)) ([134ce42](https://github.com/benthevining/BenBot/commit/134ce427e74882ddd98a9b07de6b5c252a321812))
* **eval:** endgame "mop up" score ([#120](https://github.com/benthevining/BenBot/issues/120)) ([cde915d](https://github.com/benthevining/BenBot/commit/cde915d48e208524b4d228ab4365b76bea0e4f25))
* **eval:** extra bonus for protected passed pawns ([#121](https://github.com/benthevining/BenBot/issues/121)) ([0917660](https://github.com/benthevining/BenBot/commit/09176606a7c739b9f3fe0c65bec43062f128cdab))
* **eval:** extra bonus for protected passed pawns ([#121](https://github.com/benthevining/BenBot/issues/121)) ([9973f68](https://github.com/benthevining/BenBot/commit/9973f680449675b113deca0f6476fa197a8d6a27))
* **eval:** extra penalty for isolated doubled pawns ([#121](https://github.com/benthevining/BenBot/issues/121)) ([8058186](https://github.com/benthevining/BenBot/commit/8058186b751c824e18ff6184d561011adf92f367))
* **eval:** penalty for backward pawns ([#121](https://github.com/benthevining/BenBot/issues/121)) ([c5fc921](https://github.com/benthevining/BenBot/commit/c5fc921c03a95c5c1bbeb20490dd86b287fbbbae))
* **eval:** penalty for doubled pawns ([#121](https://github.com/benthevining/BenBot/issues/121)) ([cdbc8aa](https://github.com/benthevining/BenBot/commit/cdbc8aa6a31de3e0a5e2af7144a4e57bae4312a4))
* **eval:** penalty for enemy knights/queens near your king ([#118](https://github.com/benthevining/BenBot/issues/118)) ([16aa689](https://github.com/benthevining/BenBot/commit/16aa689c88138cc61c87b9f4eac8cf118711ffd1))
* **eval:** penalty for isolated pawns ([#121](https://github.com/benthevining/BenBot/issues/121)) ([faa4b93](https://github.com/benthevining/BenBot/commit/faa4b939a5c40940b4f841e3e548c3bf875a18ea))
* **eval:** penalty for king on open file or diagonal ([#118](https://github.com/benthevining/BenBot/issues/118)) ([44f8fc1](https://github.com/benthevining/BenBot/commit/44f8fc113ee0fb2677718bf7760b46362d45dbdd))
* **eval:** penalty for king stranded in center without castling rights ([#118](https://github.com/benthevining/BenBot/issues/118)) ([6a46f08](https://github.com/benthevining/BenBot/commit/6a46f081e3cf7d23d4931d07853b93f6791e5d54))
* **eval:** scoring number of squares controlled around opponent's king ([#118](https://github.com/benthevining/BenBot/issues/118)) ([9c0cc73](https://github.com/benthevining/BenBot/commit/9c0cc733d3767c262047f2a5b02e0a8c27e7ffe9))
* **eval:** using different PST for king in endgame ([#110](https://github.com/benthevining/BenBot/issues/110)) ([8edf181](https://github.com/benthevining/BenBot/commit/8edf18158666dedaffb7dfb095173f48decc7607))
* **eval:** using knight distance to calculate penalty for enemy knights near your king ([#118](https://github.com/benthevining/BenBot/issues/118)) ([8fb8964](https://github.com/benthevining/BenBot/commit/8fb8964b50542f76a3220904b519f5364f74e1f0))
* function to detect backwards pawns ([c65f445](https://github.com/benthevining/BenBot/commit/c65f4453b550949480949327d8da58a7db7eabaa))
* function to get passed pawns ([#40](https://github.com/benthevining/BenBot/issues/40)) ([aeec23c](https://github.com/benthevining/BenBot/commit/aeec23c952aaee0a5319c88794cfdeea628542ea))
* initial commit of dumpbook command ([f94d5b2](https://github.com/benthevining/BenBot/commit/f94d5b2b16e4331f06ab69ba23a2b1a31ce86de2))
* initial commit of opening book class ([bd51d61](https://github.com/benthevining/BenBot/commit/bd51d617be8e191d5a0cca5ae7111be71af03aa4))
* initial commit of opening book data ([d2e29d4](https://github.com/benthevining/BenBot/commit/d2e29d42d09dd7780a15db6c3d06c742b77b6add))
* initial commit of text table class ([338079d](https://github.com/benthevining/BenBot/commit/338079df4193c98033c4193bfe7b607b8d402a7b))
* loadbook command now has flag to ignore variations ([dc04b11](https://github.com/benthevining/BenBot/commit/dc04b1107071eed74c71266fa30326cff406aff6))
* makenull command ([#171](https://github.com/benthevining/BenBot/issues/171)) ([37877d1](https://github.com/benthevining/BenBot/commit/37877d17660c673fb1fdaec2a9268c8b1447455e))
* num_squares_attacked() function ([6b082ad](https://github.com/benthevining/BenBot/commit/6b082ad7b7fd702fe08a436b318c7e8deb1c411f))
* opening book can now read moves from PGN ([314793f](https://github.com/benthevining/BenBot/commit/314793f302ff805cafaeb1710cea9918fb598618))
* OpeningBook::add_from_json() ([d31ceb1](https://github.com/benthevining/BenBot/commit/d31ceb17f4d7cea9942ef87c1efee653154ea09e))
* reporting number of beta cutoffs ([#178](https://github.com/benthevining/BenBot/issues/178)) ([0cfa7c6](https://github.com/benthevining/BenBot/commit/0cfa7c67ae44063dc25ed53598461b0b2ae1d4aa))
* reporting number of MDP cutoffs ([#178](https://github.com/benthevining/BenBot/issues/178)) ([7d970e1](https://github.com/benthevining/BenBot/commit/7d970e1bbea749cc9ae1cb4ee54bfbea10427c65))
* reporting number of TT hits ([#174](https://github.com/benthevining/BenBot/issues/174)) ([0a7a804](https://github.com/benthevining/BenBot/commit/0a7a804ebc3fa5f9b416d06cd0f3356fc687cf5b))
* **search:** Context::in_progress() ([#155](https://github.com/benthevining/BenBot/issues/155)) ([50d87b1](https://github.com/benthevining/BenBot/commit/50d87b177c5b7d10560c14c600a86ee118bd29a4))
* **search:** Context::wait() ([#155](https://github.com/benthevining/BenBot/issues/155)) ([99a13ca](https://github.com/benthevining/BenBot/commit/99a13ca89eeb681cbb6a612d7598c38293b33027))
* **search:** mate distance pruning ([b590506](https://github.com/benthevining/BenBot/commit/b590506c41378074e0391145c2880b82ccc5a2dd))
* **search:** printing 'mate in' UCI info strings ([#127](https://github.com/benthevining/BenBot/issues/127)) ([9151c78](https://github.com/benthevining/BenBot/commit/9151c7832259d56b1530aa835713a10da9ca4487))
* **search:** printing currmove ([#127](https://github.com/benthevining/BenBot/issues/127)) ([88b69ce](https://github.com/benthevining/BenBot/commit/88b69ce8b6787a2ea2be958ce2f23aa037eb1b9b))
* **search:** printing total nodes searched, nps ([#127](https://github.com/benthevining/BenBot/issues/127)) ([78fbbb6](https://github.com/benthevining/BenBot/commit/78fbbb68ece9a15fe09368f5cfc66657874f6e5c))
* **search:** printing UCI info for each ID iteration ([#127](https://github.com/benthevining/BenBot/issues/127)) ([2b99bf3](https://github.com/benthevining/BenBot/commit/2b99bf3233c4378b6a87af7d91719cfe54747421))
* **search:** querying opening book during search ([4e5cad4](https://github.com/benthevining/BenBot/commit/4e5cad4f07c3dbf4862c39cd52ca01aa834fb99c))
* showpos command ([9e528a5](https://github.com/benthevining/BenBot/commit/9e528a5899a8da4d4f1311ec97276b4e839471c1))
* **uci:** command to dump option values ([#164](https://github.com/benthevining/BenBot/issues/164)) ([381d398](https://github.com/benthevining/BenBot/commit/381d398249e4488f683783491f514263e2c01ca0))
* **uci:** help command ([#164](https://github.com/benthevining/BenBot/issues/164)) ([835cac6](https://github.com/benthevining/BenBot/commit/835cac6d8d357418e4e46899b1b8d928b99bfabd))
* **uci:** help output now displays version string ([#165](https://github.com/benthevining/BenBot/issues/165)) ([9204cef](https://github.com/benthevining/BenBot/commit/9204cef1a74197c162f2f981d3261d476bafa640))
* **uci:** loadbook command to load openings from a given file ([#163](https://github.com/benthevining/BenBot/issues/163)) ([956158d](https://github.com/benthevining/BenBot/commit/956158d0b4bd81d6c1c12858247835ec3f14c143))
* using text table to format options info ([90cface](https://github.com/benthevining/BenBot/commit/90cface8b79eb12b01f362f75492b935e0c4107f))

### Bug Fixes

* aborting search ([229c132](https://github.com/benthevining/BenBot/commit/229c13227c8d5c1f5b6fffab755dd2f589634fbe))
* aborting search ([e1f30e6](https://github.com/benthevining/BenBot/commit/e1f30e611535d07eb90f404ef08659503c960cfa))
* aborting search ([5dbedc5](https://github.com/benthevining/BenBot/commit/5dbedc5d863c4392f596f373ee5c7d9c18842e0c))
* aborting search during alpha/beta ([7e21771](https://github.com/benthevining/BenBot/commit/7e21771bb3b4d77d9a94690956ec8972f706c18d))
* aborting search during quiescence ([72d479e](https://github.com/benthevining/BenBot/commit/72d479ef6fd94b592fad7d62f22eb481e35b645d))
* aborting search during quiescence search ([a65b893](https://github.com/benthevining/BenBot/commit/a65b89341059e902846d349340b1417cffe8a322))
* back to std::vector for magic moves storage ([33c7f1b](https://github.com/benthevining/BenBot/commit/33c7f1b6c28c590bfdc29a88498612b4f5d48fd7))
* **book:** pruning duplicate moves in opening book ([0564b0c](https://github.com/benthevining/BenBot/commit/0564b0c95b82d34e22f2d94c1300228ea723df39))
* **eval:** bonus for connected rooks on open files ([38086e6](https://github.com/benthevining/BenBot/commit/38086e6a82a3992fd04687605c608aa64566fe84))
* **eval:** not counting defender's king when evaluating number of squares defended around king ([b35a277](https://github.com/benthevining/BenBot/commit/b35a2778859464c7551b78daa711b6d4a0ab1ed5))
* **eval:** penalty for open/stranded king is lessened in endgame ([10897f5](https://github.com/benthevining/BenBot/commit/10897f5c091c812982d5469d2d75f89786a8ce79))
* **eval:** PST game phase weighting ([736b7c3](https://github.com/benthevining/BenBot/commit/736b7c3d69fa682150e08ae0a60e55443434748d))
* MSVC constexpr fixes ([ae322d0](https://github.com/benthevining/BenBot/commit/ae322d00fcd808f01915e0e5a1d2d48d55c1aa8c))
* no longer aborting search during alpha/beta ([a2f8d8f](https://github.com/benthevining/BenBot/commit/a2f8d8f9c0d16467b2ff8b2ee0c6c9bae073277f))
* parse_all_pgns() ([0522e35](https://github.com/benthevining/BenBot/commit/0522e350857a228d70e0e07d501e65f953ccf327))
* parse_all_pgns() ([e40f945](https://github.com/benthevining/BenBot/commit/e40f945c824e65312de9694bee17cfe9ef1e175b))
* reverted search function refactor to combine regular/quiescence searches ([f2a44d1](https://github.com/benthevining/BenBot/commit/f2a44d1e6cae932a4cc7eaa4bffbe540a486921e))
* search bug with decrementing depth past 0 ([7a7bf3a](https://github.com/benthevining/BenBot/commit/7a7bf3ae622636df01e4cd87e22063dbff079b77))
* **search:** avoiding threefold reps ([404ee76](https://github.com/benthevining/BenBot/commit/404ee762e2d61cae99a2bad63c19e944064122c4))
* **search:** checkmate detection bug in quiescence search ([c9d68ee](https://github.com/benthevining/BenBot/commit/c9d68ee5a87ae27f1810e25eed9ad87930405c6a))
* **search:** max nodes limit ([a0016c9](https://github.com/benthevining/BenBot/commit/a0016c95aa9bfd7b847ac042e33eb38473b66eae))
* **search:** not storing evals in transposition table during quiescence search ([534aaad](https://github.com/benthevining/BenBot/commit/534aaad462d46ab4310cb5d24c77b7fd833e9f89))
* **search:** scaling of mate scores ([#147](https://github.com/benthevining/BenBot/issues/147)) ([28652bc](https://github.com/benthevining/BenBot/commit/28652bc5097ebaf196c5ec21554edac80b990394))
* speeding up book loading ([f945eba](https://github.com/benthevining/BenBot/commit/f945eba5c0f58f191b30d02bc2cdc772c0e23e36))
* sprt test using opening book once again ([12ce471](https://github.com/benthevining/BenBot/commit/12ce471a5efe1547e53a4cf1615af233bda3a274))
* threefold detection ([bae962b](https://github.com/benthevining/BenBot/commit/bae962b703ea4a7a1ff9ac708e0650430628c085))
* UCI info printing ([42866cf](https://github.com/benthevining/BenBot/commit/42866cf7aaa1e536a3f3b46a5dba95c1eb7f3a14))
* updating bumpversion config ([d06ee1b](https://github.com/benthevining/BenBot/commit/d06ee1b0ffa6c0c5982110dbba23d85c0a3fe7fe))
* when aborting search, only using result from last completed depth ([d9bcf57](https://github.com/benthevining/BenBot/commit/d9bcf572d285aaf8fe9f187b1e46f8c550808e9d))
* Zobrist hashing bug ([#124](https://github.com/benthevining/BenBot/issues/124)) ([5f17f8c](https://github.com/benthevining/BenBot/commit/5f17f8cd2769b710bcaa796206c8182e363572ca))
## [1.2.1](https://github.com/benthevining/BenBot/compare/v1.2.0...v1.2.1) (2025-08-15)

### Features

* --no-logo option ([ac58c83](https://github.com/benthevining/BenBot/commit/ac58c83100313118dd45f8e5dbe6faa211df00dd))
* action to tag & release ([#239](https://github.com/benthevining/BenBot/issues/239)) ([a32ca7f](https://github.com/benthevining/BenBot/commit/a32ca7f5141ea05567c53518ccbe8c90e0041006))
* ben-bot now has a perft command ([#170](https://github.com/benthevining/BenBot/issues/170)) ([850dd55](https://github.com/benthevining/BenBot/commit/850dd5595f1e28848fa88be1a393bf824e4001fa))
* command to display compiler info ([#185](https://github.com/benthevining/BenBot/issues/185)) ([874cb51](https://github.com/benthevining/BenBot/commit/874cb516f061ea1ef55841537d48b4a87f59f320))
* compiler info command now displays build configuration name ([#185](https://github.com/benthevining/BenBot/issues/185)) ([388b15e](https://github.com/benthevining/BenBot/commit/388b15e777f66872c1cc14ef7c2bb1524b991176))
* creating install rules & component for BenBot ([3f278df](https://github.com/benthevining/BenBot/commit/3f278df7f0a503118e680b18dec55486f4920169))
* deploying Github pages via actions ([82ed6f3](https://github.com/benthevining/BenBot/commit/82ed6f34f6bf2c4c94ebe2f0ba5e1f24db125475))
* function to enable UTF-8 console output on Windows ([#195](https://github.com/benthevining/BenBot/issues/195)) ([2d2bedc](https://github.com/benthevining/BenBot/commit/2d2bedce8a63eb9ec3a57e547bdaa9f85beb27ab))
* gcc preset ([948ec72](https://github.com/benthevining/BenBot/commit/948ec72b76f0d3a53b191eb5620a3540e235d252))
* implemented bench command ([#129](https://github.com/benthevining/BenBot/issues/129)) ([da89645](https://github.com/benthevining/BenBot/commit/da89645161220631185d3503b8e1c2918fc539d0))
* implemented debug mode options ([#190](https://github.com/benthevining/BenBot/issues/190)) ([5b6ff2f](https://github.com/benthevining/BenBot/commit/5b6ff2f5d0abf44dc7a44b20be3f7347370c9f08))
* initial commit of bench command ([#129](https://github.com/benthevining/BenBot/issues/129)) ([7fd1743](https://github.com/benthevining/BenBot/commit/7fd17439655f40711830f5e3d2d5bd378511a224))
* initial commit of pondering ([cdc87eb](https://github.com/benthevining/BenBot/commit/cdc87eb7d89b387be4cd76a123b776b229a73b32))
* JUnit reports on PRs ([be8a033](https://github.com/benthevining/BenBot/commit/be8a033d43a0d08341825ef59edcabb7394a7685))
* memory-mapped file class ([c9a8999](https://github.com/benthevining/BenBot/commit/c9a899992e2d3ce7ed6da539f834c4ebce0cca17))
* msvc preset ([7890418](https://github.com/benthevining/BenBot/commit/789041858c619eb7e1c07e9d576692261c23a8cb))
* parse_all_epds() function in libchess ([3825f65](https://github.com/benthevining/BenBot/commit/3825f6506996de7f9e4bf99933d2259780d97adf))
* printing board as ASCII by default, with option to print as UTF8 ([3630d64](https://github.com/benthevining/BenBot/commit/3630d64800977d51f6daea5d00f16c6e194c42b0))
* printing PV ([#159](https://github.com/benthevining/BenBot/issues/159)) ([5a824fb](https://github.com/benthevining/BenBot/commit/5a824fbccffdd59383ffab3783fc5eeb6c704d1e))
* progressive backoff function ([a9fee64](https://github.com/benthevining/BenBot/commit/a9fee6443e37b963efa5d5a830f33f98648231ac))
* progressive_backoff() function ([4883782](https://github.com/benthevining/BenBot/commit/4883782833842a5a1d63c8a99d48a9f1c87c27b2))
* uploading artifact from action ([7866b9c](https://github.com/benthevining/BenBot/commit/7866b9c4fdcb06ec7f6e75ac7d1b4706fed71d6c))
* uploading artifact from action ([1ea677b](https://github.com/benthevining/BenBot/commit/1ea677b14002ffb1e3d2a00cb575b1abbf36f9c6))

### Bug Fixes

* action to tag & release ([#239](https://github.com/benthevining/BenBot/issues/239)) ([4c43ab3](https://github.com/benthevining/BenBot/commit/4c43ab3eec82731d432cfba6c0225ba47c1070dd))
* action to tag & release ([#239](https://github.com/benthevining/BenBot/issues/239)) ([65c8b1c](https://github.com/benthevining/BenBot/commit/65c8b1cdce62d222b4a2838f739d52d027a31dbd))
* action to tag & release ([#239](https://github.com/benthevining/BenBot/issues/239)) ([6ecb584](https://github.com/benthevining/BenBot/commit/6ecb584919c440b109da312c69b9a1deeff76ff7))
* action to tag & release ([#239](https://github.com/benthevining/BenBot/issues/239)) ([5f70c5f](https://github.com/benthevining/BenBot/commit/5f70c5f235460ff85663d9969502d47a21d6f2bf))
* action to tag & release ([#239](https://github.com/benthevining/BenBot/issues/239)) ([a7acd2a](https://github.com/benthevining/BenBot/commit/a7acd2a49fea162793ab590c8c39c4466d03ecb1))
* action to tag & release ([#239](https://github.com/benthevining/BenBot/issues/239)) ([5b44228](https://github.com/benthevining/BenBot/commit/5b44228958b7184cb230e1ce38ffc2da67970669))
* action to tag & release ([#239](https://github.com/benthevining/BenBot/issues/239)) ([7ba6b3a](https://github.com/benthevining/BenBot/commit/7ba6b3aa9dd63ae08b70db00fa1f2cd5d3ddf900))
* actions ([78d2083](https://github.com/benthevining/BenBot/commit/78d2083da08dcb422b5ef7334400460fbe0931e8))
* actions ([0181e82](https://github.com/benthevining/BenBot/commit/0181e82f162572005b24b5200bd6d8830bb52814))
* actions ([8cc4ed7](https://github.com/benthevining/BenBot/commit/8cc4ed78fb0d066c8c4e844ad1446689e6f0c819))
* actions ([8450894](https://github.com/benthevining/BenBot/commit/8450894f0005330ecb981369f9b4b518093bf151))
* actions ([3719973](https://github.com/benthevining/BenBot/commit/371997341581f361e8ff9c4b3c2909e14224f667))
* actions ([7b1af37](https://github.com/benthevining/BenBot/commit/7b1af3754dc5a9aff9b454bc2faf229ed64c8726))
* actions ([781ee07](https://github.com/benthevining/BenBot/commit/781ee0716bfef4ec0209a2d71ea86c577f3a466f))
* actions ([04c2db2](https://github.com/benthevining/BenBot/commit/04c2db27af7e6ea9a5c138f5ffb4f6b8545c4d33))
* actions ([e116810](https://github.com/benthevining/BenBot/commit/e1168107899518105e7eb37278dfc03d6cb6d641))
* actions ([ab9cda8](https://github.com/benthevining/BenBot/commit/ab9cda8ccf214ad071fce3fd5c9e04792c4f8d3c))
* actions ([e53dc49](https://github.com/benthevining/BenBot/commit/e53dc496adf66e6376f333ec005320a0ee337367))
* actions ([1471d6f](https://github.com/benthevining/BenBot/commit/1471d6f556708883e255c7b7a4fc90aec7c94db3))
* actions ([aff41c9](https://github.com/benthevining/BenBot/commit/aff41c942d39881b08b2f248a17389b7e831d149))
* actions ([526ed7c](https://github.com/benthevining/BenBot/commit/526ed7cd95c6889636c08ec41a050086f0717154))
* actions ([7bf1fa6](https://github.com/benthevining/BenBot/commit/7bf1fa62583d1beeda70f9fe8babfb9c4e4246e5))
* actions ([0b8078a](https://github.com/benthevining/BenBot/commit/0b8078a449a638d5ee9a2af871894ffe4d225a7b))
* actions ([3111ada](https://github.com/benthevining/BenBot/commit/3111adad3144d2d7061cba9da4379d87f46cc3e2))
* actions ([2eb7227](https://github.com/benthevining/BenBot/commit/2eb7227960fa999bc1acb85c970d5464a3049015))
* actions ([2008a44](https://github.com/benthevining/BenBot/commit/2008a4468375199256c14cc87e9ed0ce8c711c28))
* assertion failure in position solver tests ([b5ebebe](https://github.com/benthevining/BenBot/commit/b5ebebea4e073186102b62b3369294c1b9c004f9))
* bad_alloc error in rampart tests ([#217](https://github.com/benthevining/BenBot/issues/217)) ([1f804d7](https://github.com/benthevining/BenBot/commit/1f804d7d013ecc4fbef215f0621ee1a78886e4a3))
* checking for python-chess module in cmake ([63a5d53](https://github.com/benthevining/BenBot/commit/63a5d5391f09a700708db61ebc62ab03af02ce01))
* constexpr fixes ([443ab85](https://github.com/benthevining/BenBot/commit/443ab85568b8b318473bb44a9338853767a0310f))
* cppcheck ([bbce197](https://github.com/benthevining/BenBot/commit/bbce197bf75e323531458a86c78c48d221f887ad))
* docs build ([89138b9](https://github.com/benthevining/BenBot/commit/89138b9df596f73664046bafa20221cf56cacd1d))
* docs build ([e14f543](https://github.com/benthevining/BenBot/commit/e14f543a38dcc491a3ba5a41e852df51a94ac3a4))
* engine now stores pondermove in atomic ([21e8ad9](https://github.com/benthevining/BenBot/commit/21e8ad982a2f6c7ac8b0ad5186d5a6768939b017))
* gcc preset ([223f132](https://github.com/benthevining/BenBot/commit/223f132d8f28326a644ac233e624d56ad532213a))
* implemented ponderhit command ([514cf11](https://github.com/benthevining/BenBot/commit/514cf1127d4bb528d49f04a9eb92b8861b8b2146))
* JUnit reports on PRs ([179f37c](https://github.com/benthevining/BenBot/commit/179f37c6d182e565c7447c561790ef691e06eb36))
* JUnit reports on PRs ([6b9048f](https://github.com/benthevining/BenBot/commit/6b9048f89cc07058c70ed02bfe8643ee64425ce3))
* JUnit reports on PRs ([492d8ff](https://github.com/benthevining/BenBot/commit/492d8ffb87e8bac3a1331c767825749d4859fb6b))
* JUnit reports on PRs ([8542f4b](https://github.com/benthevining/BenBot/commit/8542f4b6011045f836eb3c01ad9f034dd7eb1334))
* JUnit reports on PRs ([4423833](https://github.com/benthevining/BenBot/commit/44238338c022f887f7eb28629350b78bbc79920d))
* JUnit reports only for PRs ([fe9e21d](https://github.com/benthevining/BenBot/commit/fe9e21de670d557e40d5f18b1fd370783e443fb4))
* MSVC memory mapped file fix ([ed3643b](https://github.com/benthevining/BenBot/commit/ed3643bf7735dcea0df9ef56109a4ae440e4de1f))
* not printing PV for now (appears to be very slow) ([a6bfbf8](https://github.com/benthevining/BenBot/commit/a6bfbf858b62cad86f5dbf8298814378c82d7cfc))
* perft test script ([b319584](https://github.com/benthevining/BenBot/commit/b319584d9ba328cf3bad15a221ba19d46cee15bf))
* pre-commit ([6fc7a0d](https://github.com/benthevining/BenBot/commit/6fc7a0de0d1fc991f630a724f976369aa95a3421))
* pre-commit CI config ([12f4495](https://github.com/benthevining/BenBot/commit/12f4495b798e60c0bb4d7af5c7cd1f83034448a3))
* removed pondering for now ([71a063c](https://github.com/benthevining/BenBot/commit/71a063c1e531f59b9159d5ad5d29ced0dff7cf85))
* shrinking vectors when possible ([6d848e8](https://github.com/benthevining/BenBot/commit/6d848e84991882c904e638c653668a2112a997f1))
* specifying C++23 for CI builds ([4157674](https://github.com/benthevining/BenBot/commit/41576749fac619b67ed93e878b1f826d8e1b8ef0))
* threefold checker ([#161](https://github.com/benthevining/BenBot/issues/161)) ([1e5e944](https://github.com/benthevining/BenBot/commit/1e5e944d7c47921da58cabc4bc7218517f06414e))
* WIN32 platform detection ([c6d3401](https://github.com/benthevining/BenBot/commit/c6d34011caf9dbfa33e5c960748aab4c5cb4d67b))
* Windows build of memory mapped file class ([3ffa826](https://github.com/benthevining/BenBot/commit/3ffa826f9115103347eccc69a78de340fec1367e))
* Windows build of memory mapped file class ([4234deb](https://github.com/benthevining/BenBot/commit/4234debfff1adb7a81f5718374fc495e43a740a7))
## [1.2.2](https://github.com/benthevining/BenBot/compare/v1.2.1...v1.2.2) (2025-08-15)

### Bug Fixes

* action to tag & release ([#239](https://github.com/benthevining/BenBot/issues/239)) ([8c73744](https://github.com/benthevining/BenBot/commit/8c73744a5559c15bdd0bf8ba3ba8f4b191d2b6a8))
## [1.2.3](https://github.com/benthevining/BenBot/compare/v1.2.2...v1.2.3) (2025-08-15)

### Bug Fixes

* action to tag & release ([#239](https://github.com/benthevining/BenBot/issues/239)) ([a560903](https://github.com/benthevining/BenBot/commit/a56090327f8f431bfa7a7c5ad3f39106cab51a1a))
## [1.2.4](https://github.com/benthevining/BenBot/compare/v1.2.3...v1.2.4) (2025-08-15)

### Bug Fixes

* action to tag & release ([#239](https://github.com/benthevining/BenBot/issues/239)) ([ed5d32f](https://github.com/benthevining/BenBot/commit/ed5d32f2fcc047bab8d58f0682cf67e32e8b72fb))
## [1.2.5](https://github.com/benthevining/BenBot/compare/v1.2.4...v1.2.5) (2025-08-15)

### Bug Fixes

* action to tag & release ([#239](https://github.com/benthevining/BenBot/issues/239)) ([895616f](https://github.com/benthevining/BenBot/commit/895616fd9b6659ca7b8888914519899bce1d17c0))
* action to tag & release ([#239](https://github.com/benthevining/BenBot/issues/239)) ([eefc214](https://github.com/benthevining/BenBot/commit/eefc214b2701bba4ec949d9644531d76e0fcbf07))
## [1.2.6](https://github.com/benthevining/BenBot/compare/v1.2.5...v1.2.6) (2025-08-15)
## [1.2.7](https://github.com/benthevining/BenBot/compare/v1.2.6...v1.2.7) (2025-08-15)

### Bug Fixes

* tag & release action ([#239](https://github.com/benthevining/BenBot/issues/239)) ([c16ba13](https://github.com/benthevining/BenBot/commit/c16ba1384a20befe2621415886785277d3016794))
## [1.2.8](https://github.com/benthevining/BenBot/compare/v1.2.7...v1.2.8) (2025-08-15)

### Bug Fixes

* tag & release action ([#239](https://github.com/benthevining/BenBot/issues/239)) ([743ea25](https://github.com/benthevining/BenBot/commit/743ea25bfffa5ffa1e0c97a4b91637fcc237b6a2))
## [1.2.9](https://github.com/benthevining/BenBot/compare/v1.2.8...v1.2.9) (2025-08-15)

### Bug Fixes

* tag & release action ([#239](https://github.com/benthevining/BenBot/issues/239)) ([135ce9c](https://github.com/benthevining/BenBot/commit/135ce9c0070e35783de3c0ae45cacd96cdc04262))
## [1.2.10](https://github.com/benthevining/BenBot/compare/v1.2.9...v1.2.10) (2025-08-15)
## [1.2.11](https://github.com/benthevining/BenBot/compare/v1.2.10...v1.2.11) (2025-08-15)

### Bug Fixes

* actions ([93703f3](https://github.com/benthevining/BenBot/commit/93703f3676d55866540ab85961292a16f5e5d33b))
## [1.2.12](https://github.com/benthevining/BenBot/compare/v1.2.11...v1.2.12) (2025-08-15)

### Bug Fixes

* actions ([3c94ccb](https://github.com/benthevining/BenBot/commit/3c94ccbf4d5773d3e5b25930ca0cd44ff59e6864))
## [1.2.13](https://github.com/benthevining/BenBot/compare/v1.2.12...v1.2.13) (2025-08-15)

### Bug Fixes

* actions ([89c7715](https://github.com/benthevining/BenBot/commit/89c771504088b731e363ca7f0a01e554b09171c5))
## [1.2.14](https://github.com/benthevining/BenBot/compare/v1.2.13...v1.2.14) (2025-08-15)

### Features

* msvc build presets ([4536ff1](https://github.com/benthevining/BenBot/commit/4536ff1fa02fb6c8533aa9d4a1fa713a06af0012))

### Bug Fixes

* actions commit SHAs ([f7f01ea](https://github.com/benthevining/BenBot/commit/f7f01ea5d5767222ce1cda2cd7ac40d06d3aff0a))
* bug with Zobrist hash value being used uninitialized ([ba5e56e](https://github.com/benthevining/BenBot/commit/ba5e56e1ba0ec7a8b6f69feecb39da378ba60967))
* fixing some warnings ([d791574](https://github.com/benthevining/BenBot/commit/d791574be131de4a49c22a283731d38933adf43c))
* precommit ci ([f6aa40e](https://github.com/benthevining/BenBot/commit/f6aa40e798c9c9e841ec36aa23bc039932af62c2))
* python-chess timeouts ([#224](https://github.com/benthevining/BenBot/issues/224)) ([d9af53d](https://github.com/benthevining/BenBot/commit/d9af53d3fd3a8a0b2e89d5adab2973888fa3cd5a))
* warnings ([be585b0](https://github.com/benthevining/BenBot/commit/be585b0f5e630efa4a09d2a550950849745baa1e))
* warnings ([d995288](https://github.com/benthevining/BenBot/commit/d995288c3eb210e282c8859d921e61a6009c1c94))
## [1.2.15](https://github.com/benthevining/BenBot/compare/v1.2.14...v1.2.15) (2025-08-17)

### Bug Fixes

* actions ([e44956e](https://github.com/benthevining/BenBot/commit/e44956e8fc350d3d17aac51dfae8bf358c37b12a))
## [1.2.16](https://github.com/benthevining/BenBot/compare/v1.2.15...v1.2.16) (2025-08-17)
## [1.2.17](https://github.com/benthevining/BenBot/compare/v1.2.16...v1.2.17) (2025-08-19)
## [1.3.0](https://github.com/benthevining/BenBot/compare/v1.2.17...v1.3.0) (2025-08-19)
## [1.4.0](https://github.com/benthevining/BenBot/compare/v1.3.0...v1.4.0) (2025-09-02)

### Features

* devcontainer config ([7682a32](https://github.com/benthevining/BenBot/commit/7682a32a7ad2d142181fe231259a8840fa1b1748))
* releases now include changelog notes in release body text ([#263](https://github.com/benthevining/BenBot/issues/263)) ([24a270c](https://github.com/benthevining/BenBot/commit/24a270c02b3881d6560163ad6a273aee536b6d0a))

### Bug Fixes

* constexpr error ([d759449](https://github.com/benthevining/BenBot/commit/d759449f1f9dd89b9b4c425d3d848d7e36fb32a4))
* removed failing precommit hook ([4467e52](https://github.com/benthevining/BenBot/commit/4467e52da26d2fa6005274a35ac907f33d2f9388))
* sprt workflow ([52626ea](https://github.com/benthevining/BenBot/commit/52626ea6db19186a9b8e4c6adcc7c6e747da6c0a))
* test failure ([a8a8e95](https://github.com/benthevining/BenBot/commit/a8a8e95eb6faf61798a11b42a405627d5ac4ef05))
* threefold checker ([1058ec3](https://github.com/benthevining/BenBot/commit/1058ec361dead54c7906424ba4fee3a4ec36712c))
* threefold checker ([b837735](https://github.com/benthevining/BenBot/commit/b837735debbb098265d6a6002e8cbb11695dd939))
## [1.4.1](https://github.com/benthevining/BenBot/compare/v1.4.0...v1.4.1) (2025-09-02)

### Bug Fixes

* release action ([8b366f4](https://github.com/benthevining/BenBot/commit/8b366f4e280313e03e8d87a48995b3a212486db6))
## [1.4.2](https://github.com/benthevining/BenBot/compare/v1.4.1...v1.4.2) (2025-09-02)

### Bug Fixes

* release action ([4482301](https://github.com/benthevining/BenBot/commit/44823013859261cc264a5a052da5d1ec56f351fa))
## [1.4.3](https://github.com/benthevining/BenBot/compare/v1.4.2...v1.4.3) (2025-09-02)

### Bug Fixes

* release notes parsing script ([#263](https://github.com/benthevining/BenBot/issues/263)) ([fd31a34](https://github.com/benthevining/BenBot/commit/fd31a3465c2e8f1e738517dd5c9b138c511e532c))
## [1.4.4](https://github.com/benthevining/BenBot/compare/v1.4.3...v1.4.4) (2025-09-02)
## [1.4.5](https://github.com/benthevining/BenBot/compare/v1.4.4...v1.4.5) (2025-09-06)

### Features

* initial commit of vps deployment ([15d05f7](https://github.com/benthevining/BenBot/commit/15d05f7828c6570e4152b83c325cc2bd0f2ad3f0))
* integration with lichess-bot docker image ([48f55d3](https://github.com/benthevining/BenBot/commit/48f55d3c2f5a6fc569773329d9d2bf22095a425b))
* linking to specific Docker image tag ([#270](https://github.com/benthevining/BenBot/issues/270)) ([e67f952](https://github.com/benthevining/BenBot/commit/e67f952a74186401d6bfbe74214ef488e7b5e075))

### Bug Fixes

* Docker container [skip ci] ([041150b](https://github.com/benthevining/BenBot/commit/041150b641b21db553e4995303f27579a9ea663c))
* Dockerfile [skip ci] ([1ec5cfc](https://github.com/benthevining/BenBot/commit/1ec5cfcd0919b4b7974715ac2098b4675e6b0f76))
* executable install rules ([0b47a3c](https://github.com/benthevining/BenBot/commit/0b47a3c30848aec373d8f05e29c65946763dfc5e))
* fixed issue with fresh CMake configure attempting to copy file produced by file(GENERATE) that doesn't exist yet ([f2b313e](https://github.com/benthevining/BenBot/commit/f2b313ed0d0379a73f5234046df3621cd040d044))
* lichess deployment script ([edef22b](https://github.com/benthevining/BenBot/commit/edef22ba4beb2b1888734decbc72c0f5e74ce656))
* lichess deployment script ([3828a36](https://github.com/benthevining/BenBot/commit/3828a36a407624d58b1fdfcd7e65e42bdcc4edb0))
* lichess deployment script ([de5b0da](https://github.com/benthevining/BenBot/commit/de5b0da3cea1a0fbaac72b275a9c481a6b536135))
* lichess deployment script ([1320aac](https://github.com/benthevining/BenBot/commit/1320aac6c34801a79bc1cbbbcea62a15e3ec5826))
* lichess deployment script ([49bcd2e](https://github.com/benthevining/BenBot/commit/49bcd2e64e76e7f371c4d310538185b8d436cb0b))
* lichess deployment script ([2454354](https://github.com/benthevining/BenBot/commit/2454354aec9392b917e5b85298c0d40b6fb3f005))
* lichess deployment script ([0e8c7c3](https://github.com/benthevining/BenBot/commit/0e8c7c36ccf2cdd0ff8c51f9333e24f574574cbd))
* linking to specific Docker image tag ([#270](https://github.com/benthevining/BenBot/issues/270)) ([5e37bd9](https://github.com/benthevining/BenBot/commit/5e37bd95d2b5684dde174ef88479c63050c959a2))
* SPRT workflow ([a1365a4](https://github.com/benthevining/BenBot/commit/a1365a435b102de80b27221cc93c7b0e67989666))
* SPRT workflow ([032c164](https://github.com/benthevining/BenBot/commit/032c164e52cf2a52684af5dd71d91a99740b8107))
* SPRT workflow ([d620c16](https://github.com/benthevining/BenBot/commit/d620c169299fcd08e1419f38334c9c6075cf8c94))
* SPRT workflow ([2023a21](https://github.com/benthevining/BenBot/commit/2023a2154de746e19de8170a26302a1f217a91b6))
* SPRT workflow ([df85114](https://github.com/benthevining/BenBot/commit/df8511438cc057676ccb140a5b5b438999dde45f))
## [1.4.6](https://github.com/benthevining/BenBot/compare/v1.4.5...v1.4.6) (2025-09-06)
## [1.4.7](https://github.com/benthevining/BenBot/compare/v1.4.6...v1.4.7) (2025-09-06)

### Features

* action to stop lichess bot [skip ci] ([df55109](https://github.com/benthevining/BenBot/commit/df551099df3d7ba44a92dc2dec03e6d4b7e46464))
## [1.4.8](https://github.com/benthevining/BenBot/compare/v1.4.7...v1.4.8) (2025-09-06)

### Bug Fixes

* action to stop lichess bot [skip ci] ([51475c6](https://github.com/benthevining/BenBot/commit/51475c6649718e97b8904837dfcf1b6c3c59c1fc))
## [1.4.9](https://github.com/benthevining/BenBot/compare/v1.4.8...v1.4.9) (2025-09-06)

### Bug Fixes

* deploy lichess action secrets [skip ci] ([661f755](https://github.com/benthevining/BenBot/commit/661f755c8dfff6b47b4ca5cdb59344674c71a338))
* deploy lichess action secrets [skip ci] ([baf0e7b](https://github.com/benthevining/BenBot/commit/baf0e7bf1ea0d1873452ee6d497fc01d6284ffce))
* deploy lichess action secrets [skip ci] ([2ade955](https://github.com/benthevining/BenBot/commit/2ade9559eba8cc1b27c7dd1a632bc791709c774a))
## [1.4.10](https://github.com/benthevining/BenBot/compare/v1.4.9...v1.4.10) (2025-09-06)

### Bug Fixes

* Windows install rules [skip ci] ([8d12c24](https://github.com/benthevining/BenBot/commit/8d12c24c8162493fb405ad504bd9a20d8bb9c4d6))
## [1.4.11](https://github.com/benthevining/BenBot/compare/v1.4.10...v1.4.11) (2025-09-06)

### Bug Fixes

* Windows install rules [skip ci] ([5ed689e](https://github.com/benthevining/BenBot/commit/5ed689e38f36df63901fe94dc5d4e4de67b1099b))
## [1.4.12](https://github.com/benthevining/BenBot/compare/v1.4.11...v1.4.12) (2025-09-06)
## [1.4.13](https://github.com/benthevining/BenBot/compare/v1.4.12...v1.4.13) (2025-09-06)
## [1.4.14](https://github.com/benthevining/BenBot/compare/v1.4.13...v1.4.14) (2025-09-06)
## [1.4.15](https://github.com/benthevining/BenBot/compare/v1.4.14...v1.4.15) (2025-09-06)

### Bug Fixes

* release action [skip ci] ([7a3ee80](https://github.com/benthevining/BenBot/commit/7a3ee80906c7607d78ceb86d0cfc4b7a2c0fab92))
## [1.4.16](https://github.com/benthevining/BenBot/compare/v1.4.15...v1.4.16) (2025-09-06)

### Bug Fixes

* release action [skip ci] ([28c2473](https://github.com/benthevining/BenBot/commit/28c24738882c2f25ce082cc8f626a30a9204eced))
## [1.4.17](https://github.com/benthevining/BenBot/compare/v1.4.16...v1.4.17) (2025-09-06)

### Bug Fixes

* release action [skip ci] ([859f455](https://github.com/benthevining/BenBot/commit/859f455c4ab6a6feea9ffe7f5ef33d08f4b57e8f))
## [1.4.18](https://github.com/benthevining/BenBot/compare/v1.4.17...v1.4.18) (2025-09-06)

### Bug Fixes

* release action [skip ci] ([01a267d](https://github.com/benthevining/BenBot/commit/01a267d75f9e74393b30b4d9e5bdbfd6e39f714c))
## [1.4.19](https://github.com/benthevining/BenBot/compare/v1.4.18...v1.4.19) (2025-09-06)

### Bug Fixes

* release action [skip ci] ([d30f803](https://github.com/benthevining/BenBot/commit/d30f803b7505e8a5dc46fa3dfd53d5049cd33bea))
## [1.4.20](https://github.com/benthevining/BenBot/compare/v1.4.19...v1.4.20) (2025-09-06)

### Bug Fixes

* release action [skip ci] ([2187112](https://github.com/benthevining/BenBot/commit/21871122a950e8bacf3ab960c2b25f8ec0a48f8d))
## [1.4.21](https://github.com/benthevining/BenBot/compare/v1.4.20...v1.4.21) (2025-09-06)

### Bug Fixes

* release action [skip ci] ([3c5d633](https://github.com/benthevining/BenBot/commit/3c5d633a1a1afd6dc1d1f1b7605e0a4df6b750cc))
## [1.4.22](https://github.com/benthevining/BenBot/compare/v1.4.21...v1.4.22) (2025-09-06)

### Bug Fixes

* release action [skip ci] ([d65a344](https://github.com/benthevining/BenBot/commit/d65a3445869d3c2b090e7d48e09e59b2413fe344))
## [1.4.23](https://github.com/benthevining/BenBot/compare/v1.4.22...v1.4.23) (2025-09-06)
## [1.4.24](https://github.com/benthevining/BenBot/compare/v1.4.23...v1.4.24) (2025-09-06)

### Bug Fixes

* lichess-bot config [skip ci] ([7a9174e](https://github.com/benthevining/BenBot/commit/7a9174ee851db58e61d553e80cf05750e6de401d))
## [1.4.25](https://github.com/benthevining/BenBot/compare/v1.4.24...v1.4.25) (2025-09-07)

### Features

* id output now includes engine version ([#272](https://github.com/benthevining/BenBot/issues/272)) ([eaaf2c8](https://github.com/benthevining/BenBot/commit/eaaf2c8d40d5f6bc58e9d983bf8b68c3f0b0d79b))
## [1.4.26](https://github.com/benthevining/BenBot/compare/v1.4.25...v1.4.26) (2025-09-08)

### Features

* parameter to control max number of book moves ([#274](https://github.com/benthevining/BenBot/issues/274)) ([2f32d9a](https://github.com/benthevining/BenBot/commit/2f32d9a5001cf7b229f0901a59975f21fee2bee8))

### Bug Fixes

* CodeQL python notes ([f87774b](https://github.com/benthevining/BenBot/commit/f87774b2a2a220c3520773b065972d4049fc859d))
* cppcheck warning ([7b769e3](https://github.com/benthevining/BenBot/commit/7b769e3538cf14ca774781a8d5ec2cec06f2e59d))
* handling isready during search ([#232](https://github.com/benthevining/BenBot/issues/232)) ([c86a376](https://github.com/benthevining/BenBot/commit/c86a376713df8c98bf9a027408c6b0ddba878c7a))
* removed usage of gmtime() ([d42e2bf](https://github.com/benthevining/BenBot/commit/d42e2bf259f675e38b5cc3048288e80a725b4e7c))
* security alert ([12f98a7](https://github.com/benthevining/BenBot/commit/12f98a72e37e7555fcf0fdac2e5e9fe83e63c183))
* SPRT workflow [skip ci] ([57b31c8](https://github.com/benthevining/BenBot/commit/57b31c86057cf94937567e52020596aed91b84c8))
* using X-FEN style printing of EP squares to fix rampart test errors ([ab07017](https://github.com/benthevining/BenBot/commit/ab0701737ad5ca19214427a71586958d3a1c96a6))
* Windows build error ([3ab5f9e](https://github.com/benthevining/BenBot/commit/3ab5f9ee70d7b529b5191227f5ea57a65be26fc4))
* Windows build error ([a19d7e2](https://github.com/benthevining/BenBot/commit/a19d7e27b38c560d0060d36c84daa722d3f90d31))
## [1.4.27](https://github.com/benthevining/BenBot/compare/v1.4.26...v1.4.27) (2025-09-13)

### Bug Fixes

* github workflows ([a7f16c7](https://github.com/benthevining/BenBot/commit/a7f16c743215f6afa9efdc733ab3d1c5424cc925))
* removed spurious warning flags ([dde3e09](https://github.com/benthevining/BenBot/commit/dde3e09cc7b9ca721181f2035d4c796e9e9f829f))
* SPRT workflow [skip ci] ([a7387ab](https://github.com/benthevining/BenBot/commit/a7387abbda64683793a58d4f1fc4d682ce21d178))
* SPRT workflow [skip ci] ([d0d3513](https://github.com/benthevining/BenBot/commit/d0d3513ebf2e95d6dc7a29e8d7669f5084bf460d))
* SPRT workflow [skip ci] ([6de4b34](https://github.com/benthevining/BenBot/commit/6de4b349d58131256b998ae5305309bef1a1bbea))
* warning flags ([162bfe2](https://github.com/benthevining/BenBot/commit/162bfe227ce30c3f50fc494f1873f086aacdeda0))
* workflows ([f08eb44](https://github.com/benthevining/BenBot/commit/f08eb44652b300e0c40feb04dd46da9fdc513074))
## [1.4.28](https://github.com/benthevining/BenBot/compare/v1.4.27...v1.4.28) (2025-09-13)

### Bug Fixes

* bump version workflow [skip ci] ([5ae9395](https://github.com/benthevining/BenBot/commit/5ae939570802e3429ef48bc544129bb6ac75e14b))
## [1.5.0](https://github.com/benthevining/BenBot/compare/v1.4.28...v1.5.0) (2025-09-17)

### Features

* clang-tidy integration ([c480092](https://github.com/benthevining/BenBot/commit/c48009259bad6d774ba1cbca4c60a6697098be8e))
* cppcheck integration ([4a06186](https://github.com/benthevining/BenBot/commit/4a0618613d49dee89450e6299f8ab685563c47c1))
* cpplint integration ([4e8e89f](https://github.com/benthevining/BenBot/commit/4e8e89f2a2f4a7c2965a70c27778a1a41bb57e09))
* IWYU integration ([61b2c92](https://github.com/benthevining/BenBot/commit/61b2c9211246a7e4bacebba2e8beffc4061e63fd))

### Bug Fixes

* actions ([a875512](https://github.com/benthevining/BenBot/commit/a8755123dde9a716e0dc9e70ce894ce4553470f2))
* disabled clang-tidy integration ([635d49c](https://github.com/benthevining/BenBot/commit/635d49cb038d934d230b971026ffa9280a972dfc))
* get_name() now returns a string instead of a string_view ([c559bb6](https://github.com/benthevining/BenBot/commit/c559bb642797bdda44d345921f26afc4bc9d0305))
* get_name() now returns a string instead of a string_view ([6a018d1](https://github.com/benthevining/BenBot/commit/6a018d1459df2e50545ef974877f38d9e5967371))
* warning ([831b9b8](https://github.com/benthevining/BenBot/commit/831b9b80411b4472a96108b3944fdb1207d7459b))
* warnings ([091f567](https://github.com/benthevining/BenBot/commit/091f567f14768e4ab2d3054d33bbb47652de4d22))
* warnings ([d8e15eb](https://github.com/benthevining/BenBot/commit/d8e15ebea009bfab04f9cf679890c8af51b86c10))
* warnings ([4a86e95](https://github.com/benthevining/BenBot/commit/4a86e95e0ff84a448ec021b0f03c08b2215dcc4d))
## [1.5.1](https://github.com/benthevining/BenBot/compare/v1.5.0...v1.5.1) (2025-09-17)

### Bug Fixes

* Dockerfile ([1ce5dc8](https://github.com/benthevining/BenBot/commit/1ce5dc8986c569e71188733812fcc37fcdfc2a4a))
## [1.5.2](https://github.com/benthevining/BenBot/compare/v1.5.1...v1.5.2) (2025-09-18)

### Features

* engine now has a ponder option ([07e736d](https://github.com/benthevining/BenBot/commit/07e736d169d13056fab55129f0882e7281559bfe))

### Bug Fixes

* build error ([7d2671b](https://github.com/benthevining/BenBot/commit/7d2671b637a24d59d83a0a38295ddebea6a2833c))
* pondering fixes ([f7ad331](https://github.com/benthevining/BenBot/commit/f7ad3318fc5e39105038325003af7bb2e887357c))
* when in ponder mode, search isn't exited until stop or ponderhit ([3bfa629](https://github.com/benthevining/BenBot/commit/3bfa62973815d707c545cfc712f2423ecd2f97ca))
## [1.6.0](https://github.com/benthevining/BenBot/compare/v1.5.2...v1.6.0) (2025-09-28)

### Features

* bench threads now print search iteration info when in debug mode ([#305](https://github.com/benthevining/BenBot/issues/305)) ([9447152](https://github.com/benthevining/BenBot/commit/94471522822342b276aed740e870422fbee199fe))
* EngineBase can now respond to register command ([#299](https://github.com/benthevining/BenBot/issues/299)) ([38fdd83](https://github.com/benthevining/BenBot/commit/38fdd832ac55c31009fe5653faa85844b923ca78))
* EngineBase now has function to react to option change ([3bbe427](https://github.com/benthevining/BenBot/commit/3bbe427619b4ee92b58c60fe27973049ea56b476))
* flip command ([#291](https://github.com/benthevining/BenBot/issues/291)) ([de2c2bb](https://github.com/benthevining/BenBot/commit/de2c2bb2b17614c6e9d8ee36ebf03c37c7c79ed2))
* function to check if position is legal ([#293](https://github.com/benthevining/BenBot/issues/293)) ([0dc082d](https://github.com/benthevining/BenBot/commit/0dc082d0669bc0f7343fcae7daec4382fa44fb64))
* horizontal slip function ([#291](https://github.com/benthevining/BenBot/issues/291)) ([e42b950](https://github.com/benthevining/BenBot/commit/e42b95008033522767bfbd8a8a723ee866ada369))
* initial commit of page-aligned allocation functions ([60dc0a7](https://github.com/benthevining/BenBot/commit/60dc0a7129262ad17dcbe52a034266bc7a62b9a4))
* libbenbot now provides default UCI printer callbacks ([#305](https://github.com/benthevining/BenBot/issues/305)) ([244f4c1](https://github.com/benthevining/BenBot/commit/244f4c1ca3b15480bba527a0753717aae558a144))
* mul_hi64() function ([#199](https://github.com/benthevining/BenBot/issues/199)) ([106312c](https://github.com/benthevining/BenBot/commit/106312c4634925d35b0ec4b455144b7f4442688d))
* page-aligned allocation functions ([574e4e0](https://github.com/benthevining/BenBot/commit/574e4e04d29df62e2e620db7a723e4acd7d1ee64))
* PageAlignedVector class ([9c718ec](https://github.com/benthevining/BenBot/commit/9c718ecdb6da40adbc408068d085b3f30cbb6537))
* Position::flip() method ([#291](https://github.com/benthevining/BenBot/issues/291)) ([0a92412](https://github.com/benthevining/BenBot/commit/0a92412b461223a1fecc5d6203b388de71012f62))
* prefetch function ([#199](https://github.com/benthevining/BenBot/issues/199)) ([5dbbdf7](https://github.com/benthevining/BenBot/commit/5dbbdf7205647fc6cd7500304e43def3e8608cfc))
* providing PGN NAG constants ([#306](https://github.com/benthevining/BenBot/issues/306)) ([6af3eb0](https://github.com/benthevining/BenBot/commit/6af3eb06fc9a16f20a1648e877626f206e184717))
* providing PGN NAG constants ([#306](https://github.com/benthevining/BenBot/issues/306)) ([3bd4ae8](https://github.com/benthevining/BenBot/commit/3bd4ae8ba4a5442d80fced06e73ccb25c83d8bcb))
* TranspositionTable age tracking ([0aaf95d](https://github.com/benthevining/BenBot/commit/0aaf95da6d020d9122c3c8b83ffd80fa031dc914))
* UCI `best_move()` printing function ([#305](https://github.com/benthevining/BenBot/issues/305)) ([72a3967](https://github.com/benthevining/BenBot/commit/72a3967d84b20083ed7228ba59502114d5f696fe))
* UCI `info_string()` function ([#305](https://github.com/benthevining/BenBot/issues/305)) ([15ec750](https://github.com/benthevining/BenBot/commit/15ec750a8b28b6faf7a293dc431f1c00f2626fef))
* **uci:** reporting hashfull ([2393239](https://github.com/benthevining/BenBot/commit/2393239cec2d36c85e3b4fdb8153c9f1cefaad81))

### Bug Fixes

* "register name" value can now contain spaces ([#299](https://github.com/benthevining/BenBot/issues/299)) ([a0a996d](https://github.com/benthevining/BenBot/commit/a0a996d286c460be41c21b595b053ec871feca23))
* build ([450c01e](https://github.com/benthevining/BenBot/commit/450c01e39a4beed6a0e7d2f4ba32f496ebeb35d3))
* build error ([7b401be](https://github.com/benthevining/BenBot/commit/7b401be836939d8965394e1f57541b9b59b6e951))
* build error ([34a367a](https://github.com/benthevining/BenBot/commit/34a367ac710a8241efa9d6fe87215b36c536439a))
* build errors from cherry pick ([42e52a8](https://github.com/benthevining/BenBot/commit/42e52a8e4f6580427f6039ae5bd2501e4a71514f))
* checking for null moves when parsing UCI position command ([35ddef9](https://github.com/benthevining/BenBot/commit/35ddef96028ce314668ae4dc6200653e07f1b156))
* cmake ([4aa29fc](https://github.com/benthevining/BenBot/commit/4aa29fc997db52d93ff591e79d71b0105ef3b2bb))
* CTest config file ([87da1dc](https://github.com/benthevining/BenBot/commit/87da1dcf9f59498a5da00bae267e0fbd310c5038))
* delayed initialization for search callbacks ([#305](https://github.com/benthevining/BenBot/issues/305)) ([9023c4d](https://github.com/benthevining/BenBot/commit/9023c4d6a31d59a777d9126dbe5c518be8b5719f))
* final info string is no longer printed twice ([99be9db](https://github.com/benthevining/BenBot/commit/99be9db348073ff3c507b3656b6401b0acdf40bc))
* inplace_vector breaking change ([98c2c07](https://github.com/benthevining/BenBot/commit/98c2c075af89fb75cfd12573dd62be0ec7e75816))
* linter warnings ([6904ca2](https://github.com/benthevining/BenBot/commit/6904ca271e998c0b6414f43e98060a662a961fdb))
* linter warnings ([9024e1e](https://github.com/benthevining/BenBot/commit/9024e1ee19ea2c69490f9bfb39abb5e6167e1cda))
* Linux CI ([3bd9bf5](https://github.com/benthevining/BenBot/commit/3bd9bf564e1e05dafa40ca5decf8465ea60b2da3))
* Linux CI ([0cd9fa1](https://github.com/benthevining/BenBot/commit/0cd9fa190e8f3ae6ca9fa62f9d35cbc0354c5302))
* move packing ([ebc63f5](https://github.com/benthevining/BenBot/commit/ebc63f5e8ba553d93236f8c1a56ffa089204a724))
* move packing ([7a3a7f6](https://github.com/benthevining/BenBot/commit/7a3a7f6c198c1e80cbd4c3ca15845a9a86c0c4c4))
* move packing ([04f7cb7](https://github.com/benthevining/BenBot/commit/04f7cb7d2b1b2a6b14dbf35cc3b3535b37ba8ce9))
* move packing ([c4773d9](https://github.com/benthevining/BenBot/commit/c4773d963e27de3f147a369bcd84e1e0be162460))
* move packing ([38c35a1](https://github.com/benthevining/BenBot/commit/38c35a15ec10b4e8d3b465d41623823760b05c6f))
* move packing ([219b811](https://github.com/benthevining/BenBot/commit/219b8114808b3affe01b81b70c4387a006149bcc))
* MSVC install step ([f379c16](https://github.com/benthevining/BenBot/commit/f379c166c2b4d536f482c0ba18c39201b9cc2115))
* no longer terminating when invalid position set ([9c603da](https://github.com/benthevining/BenBot/commit/9c603da01814c21bf2b43eaf8c64413d59e92acd))
* no longer terminating when invalid position set ([bb8bfef](https://github.com/benthevining/BenBot/commit/bb8bfef9d82059e4384319c2d3b2f86683dea5db))
* printing of extra info when nodes searched is 0 ([81dc2d2](https://github.com/benthevining/BenBot/commit/81dc2d292c1b2ab2b199be5512142943fc2e30a1))
* remaining throw() in `util::find_matching_close_paren()` ([423e0a6](https://github.com/benthevining/BenBot/commit/423e0a64170295235a3fa4502b4619dd2d1e2ee0))
* removed build-time static analysis integration ([f0580d0](https://github.com/benthevining/BenBot/commit/f0580d02c8c59cbc09f913945865a166fcd0b415))
* specifying -stdlib=libc++ for clang ([c1cfdb7](https://github.com/benthevining/BenBot/commit/c1cfdb7ea589270465cbae00e050df489d7329c5))
* warning ([8e63075](https://github.com/benthevining/BenBot/commit/8e63075cc0234753396d93c6a4a4aaea7d3dbefa))
* warnings ([a1cdd29](https://github.com/benthevining/BenBot/commit/a1cdd29cfdf1290d7e594ac7c68ea915df5703c5))
* warnings ([0913a41](https://github.com/benthevining/BenBot/commit/0913a411de68b9bf1c7c0162fb6c95082557670a))
* warnings ([812f787](https://github.com/benthevining/BenBot/commit/812f787097e4ebb36fccc063ebcaed84dc0931a5))
* warnings ([a28a288](https://github.com/benthevining/BenBot/commit/a28a288f9c2918107dda7f57c62dc96e7e854903))
* warnings ([774294a](https://github.com/benthevining/BenBot/commit/774294aed67e56b0cfe3fada2949b73b8e28734b))
* warnings ([483e51b](https://github.com/benthevining/BenBot/commit/483e51bf12973a65d0113a023285f43f1e88cf42))
* Windows build of large page alloc functions ([f07cefc](https://github.com/benthevining/BenBot/commit/f07cefcd4720693275286b39819dc6442b05f9d6))
* Windows install ([579a7d9](https://github.com/benthevining/BenBot/commit/579a7d92f28f9414853cb59a3f566cf7ce2afccd))
## [1.6.1](https://github.com/benthevining/BenBot/compare/v1.6.0...v1.6.1) (2025-09-28)

### Bug Fixes

* **ci:** installing libc++ where needed ([2076110](https://github.com/benthevining/BenBot/commit/2076110676caa3cdf5f83c7979512202bd828ced))
## [1.6.2](https://github.com/benthevining/BenBot/compare/v1.6.1...v1.6.2) (2025-09-28)
## [1.7.0](https://github.com/benthevining/BenBot/compare/v1.6.2...v1.7.0) (2025-10-09)

### Features

* initial commit of file logger ([#347](https://github.com/benthevining/BenBot/issues/347)) ([be2d690](https://github.com/benthevining/BenBot/commit/be2d690bdf257c3281faf6c85f879c3987cb0954))
* initial commit of killer moves array ([8c5db4a](https://github.com/benthevining/BenBot/commit/8c5db4a7d0899f277486ab38b9b315439ff321ca))
* initial commit of principal variation search ([#143](https://github.com/benthevining/BenBot/issues/143)) ([6ffef74](https://github.com/benthevining/BenBot/commit/6ffef74ca3a69a30b54f25a47f969120c05e31b5))
* initial commit of printing PV ([464b0b0](https://github.com/benthevining/BenBot/commit/464b0b0fcc3a293512efbc5f6154680655da3f9e))
* printing PV ([1b27bac](https://github.com/benthevining/BenBot/commit/1b27bacc203bd1cae63c8fa1f3fcf3aabad8ba32))
* reporting number of static evaluations in search info ([f7aed8b](https://github.com/benthevining/BenBot/commit/f7aed8bec1d0d42117a1a187607e55aa12e4b2a1))
* reporting seldepth ([#328](https://github.com/benthevining/BenBot/issues/328)) ([83905bf](https://github.com/benthevining/BenBot/commit/83905bfa85fafe3e12cee6343d1519ef38a51103))
* reverse futility pruning ([b604acd](https://github.com/benthevining/BenBot/commit/b604acd7019ae41eaae93390a20fc08ec607f78b))
* supporting "mate in X" search bound ([#304](https://github.com/benthevining/BenBot/issues/304)) ([f960a95](https://github.com/benthevining/BenBot/commit/f960a95cf49010e6d52c08735b2e959b6b870aca))
* supporting Move Overhead option ([#334](https://github.com/benthevining/BenBot/issues/334)) ([dee0f48](https://github.com/benthevining/BenBot/commit/dee0f4822590d664d5ec5289f89ac57443ab3a03))
* UCI option to set debug log file path ([#347](https://github.com/benthevining/BenBot/issues/347)) ([4e13380](https://github.com/benthevining/BenBot/commit/4e133804bee393bc382b03c29f7392b0cf4c6277))

### Bug Fixes

* actions config error ([2da423b](https://github.com/benthevining/BenBot/commit/2da423b80930b3d2b150aab4406ae4a16c89ce2f))
* better supporting infinite searches ([9f0ad2c](https://github.com/benthevining/BenBot/commit/9f0ad2cdd08f4b1637e2305ba76b50004b696f58))
* build error on systems where `std::vector` is missing `append_range()` ([42d66c7](https://github.com/benthevining/BenBot/commit/42d66c7b7cba693171992445ddabaa0576f6d12d))
* calling std::cout.flush() instead of fflush(stdout) ([#345](https://github.com/benthevining/BenBot/issues/345)) ([c997ec7](https://github.com/benthevining/BenBot/commit/c997ec7eb891b2299dc99cd51c42f50cd6dd4c00))
* disabled checking legality of position ([ec2f86f](https://github.com/benthevining/BenBot/commit/ec2f86ff4c94805fbf69f7c5244653d2a5dccc2e))
* eval value range for int16_t ([d3d545c](https://github.com/benthevining/BenBot/commit/d3d545cd7ceb8f0dfda5bde69447a9be9a1808dc))
* flushing cout after each search iteration info output ([1d93ffd](https://github.com/benthevining/BenBot/commit/1d93ffd8654f82fc4511a0558e9372d5c2eb72e1))
* integer conversions ([9609d5d](https://github.com/benthevining/BenBot/commit/9609d5d246326f284b02afa182c7a64bec866a22))
* killer moves ([2b32678](https://github.com/benthevining/BenBot/commit/2b32678d1faae3ed70948fee60e00fb8f7e2bb91))
* linter warnings ([45d0855](https://github.com/benthevining/BenBot/commit/45d0855e95f3c708b96c5bb412673f44112bfbed))
* mate score handling within search ([065ce22](https://github.com/benthevining/BenBot/commit/065ce22e012c64f016d38959357e57482946a7e4))
* matetrack test config ([b8acb54](https://github.com/benthevining/BenBot/commit/b8acb54ca8f0065083858b0c6cef6125eff2a013))
* node count reporting ([77eee75](https://github.com/benthevining/BenBot/commit/77eee7525f1e23f250ffec3b8350f8e5f625a4f1))
* not using search result if search was cancelled ([92b8ad0](https://github.com/benthevining/BenBot/commit/92b8ad0510e21741630d452af566dee183362801))
* principal variation search ([9e5ed06](https://github.com/benthevining/BenBot/commit/9e5ed069cfbee9e0b820bb12d51ce4ad9baaa289))
* printing PV ([7967c97](https://github.com/benthevining/BenBot/commit/7967c9740041338f1ce0b65ebc108fcb7a77aa67))
* printing PV ([cc2a01f](https://github.com/benthevining/BenBot/commit/cc2a01ff254252f2c9abf62008752169cde29950))
* PVS in root search only, not recursive alpha/beta ([890cf54](https://github.com/benthevining/BenBot/commit/890cf54721f4a44909c124e8ed421f4426d5f798))
* removed `sync_with_stdio(false)` ([#345](https://github.com/benthevining/BenBot/issues/345)) ([3d422ac](https://github.com/benthevining/BenBot/commit/3d422acf3d484c12b304f94072cb0c5aa25a1d5d))
* removed initial logo printing ([0e3f3fa](https://github.com/benthevining/BenBot/commit/0e3f3fadfe6dee6d6d8016887285084d79f722ef))
* Score::from_tt() ([a497385](https://github.com/benthevining/BenBot/commit/a4973852623ffa50b4e464d29ebea2322ff44ce7))
* stored TT depth values for draws & mates ([263a257](https://github.com/benthevining/BenBot/commit/263a2575e1502f07f190a4bc0c310ebce219ad9f))
* storing killer moves in vectors ([cf7135f](https://github.com/benthevining/BenBot/commit/cf7135f58f819b701db064970c450ba8968db8e4))
* **TT:** explicitly checking if entries are occupied, not relying solely on key match ([55994c0](https://github.com/benthevining/BenBot/commit/55994c0f5984de72e5ddd7520d1d1ca0771cec6e))
* UCI printing now uses iostreams explicitly ([7fe8b2b](https://github.com/benthevining/BenBot/commit/7fe8b2b62514a33357e1ba4fe71ed559c3f513ee))
## [1.7.1](https://github.com/benthevining/BenBot/compare/v1.7.0...v1.7.1) (2025-10-15)

### Features

* build time can now be optionally displayed, or just date ([9b949d4](https://github.com/benthevining/BenBot/commit/9b949d401827710596d71249ac691b6e4728d79d))
* cmake option BENBOT_ALWAYS_REBUILD_RESOURCES ([da19017](https://github.com/benthevining/BenBot/commit/da19017e5d20802261de26250a0028002a21dee3))

### Bug Fixes

* constexpr build error ([06800f7](https://github.com/benthevining/BenBot/commit/06800f7c273ebfb81f05e356c570c2f429ce9bcb))
* constexpr build error ([67b2656](https://github.com/benthevining/BenBot/commit/67b2656b70ab31800e673f6d33b97985c504eb44))
* guarding against search going deeper than MAX_PLY ([#379](https://github.com/benthevining/BenBot/issues/379)) ([0791db4](https://github.com/benthevining/BenBot/commit/0791db406040193dd338859877128672261e2126))
* lichess bot config [skip ci] ([34325b5](https://github.com/benthevining/BenBot/commit/34325b509415477219285ed2fedbb7a08556a487))
* making sure build time is always correctly updated ([#384](https://github.com/benthevining/BenBot/issues/384)) ([1f7512c](https://github.com/benthevining/BenBot/commit/1f7512c94a84c60310e1dcd9669be827a8837b4e))
* pre-commit CI config ([eaed060](https://github.com/benthevining/BenBot/commit/eaed0603807213c63fe5fb5ff65cedcab9415cf4))
## [1.7.2](https://github.com/benthevining/BenBot/compare/v1.7.1...v1.7.2) (2025-10-15)

### Bug Fixes

* Docker build [skip ci] ([6f08773](https://github.com/benthevining/BenBot/commit/6f087735a0138ca52582074fb7940b94da3627a1))
## [1.7.3](https://github.com/benthevining/BenBot/compare/v1.7.2...v1.7.3) (2026-03-05)

### Features

* added dependency injection pattern for pretty printing PV moves ([#440](https://github.com/benthevining/BenBot/issues/440)) ([1b5042d](https://github.com/benthevining/BenBot/commit/1b5042dab11418fc7be0a45b8b88c807611a77b6))
* added dependency injection pattern for pretty printing PV moves ([#440](https://github.com/benthevining/BenBot/issues/440)) ([23215b4](https://github.com/benthevining/BenBot/commit/23215b4ff206c20d9a09dbaa3067d6641168ae88))
* building with IPO if supported ([#394](https://github.com/benthevining/BenBot/issues/394)) ([7976ffb](https://github.com/benthevining/BenBot/commit/7976ffbdd25882f865e4402f84b4f4cffec13479))
* code signing support ([#398](https://github.com/benthevining/BenBot/issues/398)) ([47292b8](https://github.com/benthevining/BenBot/commit/47292b88a6cd7b84dcd7c1a7ee54afe77ef7e038))
* color printing in pretty output ([2be8e23](https://github.com/benthevining/BenBot/commit/2be8e23b35cbebb6de885f4f3f5b3e2c0ddfff25))
* initial commit of ICCF notation functions ([2fa6291](https://github.com/benthevining/BenBot/commit/2fa62916c30f53b878f8cf1ac3b6e6d75794e6f8))
* initial commit of pretty printing depth & time ([da60d9a](https://github.com/benthevining/BenBot/commit/da60d9a0928d32af509c95ec308e47ea70dd7512))
* initial commit of termcolor dependency ([#385](https://github.com/benthevining/BenBot/issues/385)) ([8ae96f4](https://github.com/benthevining/BenBot/commit/8ae96f45c0d8c46d3b5d7c995548bacac5a4edd4))
* move format UCI parameter ([#440](https://github.com/benthevining/BenBot/issues/440)) ([23c88f8](https://github.com/benthevining/BenBot/commit/23c88f8ce634e2dc292fa011b632a21b0d8beda3))
* position printing now uses some color ([#385](https://github.com/benthevining/BenBot/issues/385)) ([14f51c2](https://github.com/benthevining/BenBot/commit/14f51c2239673e12d1910c5734bc768d7bb33841))
* pretty printing hashfull ([0eaf8f2](https://github.com/benthevining/BenBot/commit/0eaf8f220967ec5bafe1bab04a510a0a8d111255))
* pretty printing NPS ([7913f41](https://github.com/benthevining/BenBot/commit/7913f41fd8b0b0f0e37df67eca1f724906682080))
* pretty printing number of nodes ([5ab8c31](https://github.com/benthevining/BenBot/commit/5ab8c31c5fb114179a46a9457569b225a54c9a13))
* pretty printing PV ([a12b241](https://github.com/benthevining/BenBot/commit/a12b2411cc6f7166290285ede010ebe3b7346915))
* pretty printing score ([78c7df9](https://github.com/benthevining/BenBot/commit/78c7df911d84c7c75643cf090452e678d89a0a71))
* printing logo in color ([#385](https://github.com/benthevining/BenBot/issues/385)) ([925f125](https://github.com/benthevining/BenBot/commit/925f1256e872461fcfae97be83241d9968e53859))
* printing table header ([f8df078](https://github.com/benthevining/BenBot/commit/f8df0780aab956f67287c2532f62e1af71ee88ae))
* printing text tables using color ([#385](https://github.com/benthevining/BenBot/issues/385)) ([5f676f8](https://github.com/benthevining/BenBot/commit/5f676f813a7e8ba5fa43b506e130d0d509b4e956))
* UCI option to switch between pretty/UCI printing ([0f713f1](https://github.com/benthevining/BenBot/commit/0f713f18870fb6bb1efea130eb49b80cdbf989a6))

### Bug Fixes

* better text colors ([385ea56](https://github.com/benthevining/BenBot/commit/385ea566c6f6770c17cd689502aae381e70eb493))
* checking __cpp_lib_hardware_interference_size feature macro ([d45b5f0](https://github.com/benthevining/BenBot/commit/d45b5f002c6f3031dba8419ac12c0c659c8b713f))
* **ci:** installing cmake 4.0 ([b618940](https://github.com/benthevining/BenBot/commit/b618940fbbb0d57eba19fcbd04bf90fb5e9a7fa7))
* **ci:** installing cmake 4.0 ([#408](https://github.com/benthevining/BenBot/issues/408)) ([d03ff41](https://github.com/benthevining/BenBot/commit/d03ff41085f00e81d16f9623c682bada9a1bb809))
* cmake ([996f3c7](https://github.com/benthevining/BenBot/commit/996f3c7b1cfc5db08d1188a3745e53f9773dcf1f))
* **docs:** Doxygen warnings ([329b9c3](https://github.com/benthevining/BenBot/commit/329b9c37d56ece9e227aada23a6f5137030d5625))
* linter warnings ([3c221f7](https://github.com/benthevining/BenBot/commit/3c221f7954533668e57c7e22b62725b082743869))
* linter warnings ([ea78abd](https://github.com/benthevining/BenBot/commit/ea78abd137603a746bd9da179cfeb3c7ade7e6d4))
* missing include ([2da3618](https://github.com/benthevining/BenBot/commit/2da361847c1309d21c8333ed3d4b92425a8762d0))
* missing include ([fec78d5](https://github.com/benthevining/BenBot/commit/fec78d5a4871a83f2e5cae3e05e4f8c2f182fa46))
* MSVC build error ([32352dd](https://github.com/benthevining/BenBot/commit/32352dd48dd588b95ba3b9591f52e5a44e4dfc53))
* MSVC build error ([8a7da06](https://github.com/benthevining/BenBot/commit/8a7da06de9295f765d8238bdaae1465deb8b1e9a))
* reporting aggregate JUnit results ([#451](https://github.com/benthevining/BenBot/issues/451)) ([d43e08b](https://github.com/benthevining/BenBot/commit/d43e08b35dbaf92dedc92de7df1ac820e5c8ed13))
* SPRTs are now only manually triggered ([#403](https://github.com/benthevining/BenBot/issues/403)) ([d25350c](https://github.com/benthevining/BenBot/commit/d25350c81e9a8063934b4534198676629896437b))
* **test:** workaround for rampart's inconsistent handling of EP squares ([#392](https://github.com/benthevining/BenBot/issues/392)) ([7079201](https://github.com/benthevining/BenBot/commit/7079201ad5045cf6d76e6b3b73b99144b22fd52e))
* using inline constexpr for global constants ([3bc916d](https://github.com/benthevining/BenBot/commit/3bc916dc6d9e0b5d78aae214a71797e992fcfd91))
