# Feature Specification: Tension Othello (全く新しいオセロ)

**Feature Branch**: `001-new-othello`

**Created**: 2026-10-13

**Status**: Ready for planning

**Input**: User description: "これまでにない全く新しいオセロを開発してください。"

## Clarifications

### Session 2026-10-13

- Q: "新しさ"はどの層で定義される? → A: 核心ルール革新(石・置き・裏返し・勝敗条件そのものが既存オセロと異なる)
- Q: 核心ルール革新の主要な軸はどれ? → A: 裏返し(キャプチャ)機構
- Q: キャプチャ機構の具体形はどれ? → A: 遅延キャプチャ(挟んだ後経過時間で裏返る)。挟まれている石の数が多いほど加速し、置いた石から遠い石ほど加速しづらい。
- Q: 「経過時間」はターン制か実時間か? → A: 実時間(1秒周期で綱引きが進む)。
- Q: v1の提供形態と対戦相手は? → A: Windows .exe(C++/DxLib)、対戦相手はAIのみ。
- Q: 石を置ける条件? → A: 通常のオセロと同じ(挟みを作るマスにのみ置ける)。
- Q: 両陣営に挟まれているときは? → A: 張力メーター(白=0〜黒=100)。どちらかに振り切れると反転。
- Q: 張力メーターの初期値? → A: 中立(50)スタート(片側挟みも50スタート)。
- Q: 両陣営から同時に押されている時のメーターの動き? → A: 正味(差分)。綱引き。
- Q: 圧力周期は? → A: 1秒ごとに、アクティブ線の圧力値 p そのものを加算(誰も置かない秒にも進む)。
- Q: 圧力(N,k)は線作成時点で固定? → A: 毎秒再計算(その時最も近い自石からの距離で計算し直す)。
- Q: 同一陣営の複数線の圧力? → A: 合算(sum)。線が多いほど速く振り切れる。
- Q: contest石の全線が破れた(救出)場合? → A: contestのまま(メーター凍結、再度挟まれれば圧力再開)。救出=一時停止。
- Q: 線の遠端石が反転したら? → A: 線は破れる(遠端が自陣営でなくなったら圧力停止)=救出と同じ扱い。
- Q: 反転した石は再度contestに入れる? → A: 反転石は固定。一度振り切れば終局まで確定。
- Q: contest石は線形成の「自陣営の石」になる? → A: はい、元の色(=contest中の陣営)の石として使える。
- Q: 連鎖は配置時だけ? 発火が新連鎖を生む? → A: 配置時のみ(決定論的、テスト容易)。
- Q: 盤サイズ? → A: 標準8x8。
- Q: 开局(初期配置)? → A: 既存オセロの4石スタート(白黒2石ずつ、対称)。
- Q: CD(クールダウン)は独立か交互手番か? → A: 両陣営独立CD(同時に置ける、リアルタイム綱引き)。
- Q: CDの長さ? → A: 固定秒数(5秒)— 全ゲーム同じ。
- Q: CDが解けた陣営に置ける手がない時は? → A: 手番制のオセロと同じくスキップ(パス)。
- Q: 固定石(反転済み)だけの挟みは合法手か? → A: 合法だが圧力なし(no-op手)。CDごとに許す。
- Q: 両陣営が同じマスに同時に置こうとしたら? → A: 衝突は起こらない(処理順で確定、テスト可能)。
- Q: 両陣営CDが同じ秒に解けた時の処理順? → A: 黒優先(毎秒の処理順を黒→白に固定)。
- Q: 人間の置けるタイミング? → A: CD解済みならいつでもクリックで置ける(置いた瞬間CD開始)。
- Q: 人間プレイヤーの陣営? → A: 固定:人間=黒(先手)。
- Q: AIの着手タイミング? → A: 人間らしい思考遅延(0.5〜2秒考えてから置く)。
- Q: AIの着手は決定論的? → A: 決定論的(同じ盤面→同じ着手)。2手先のlookaheadを含む。思考遅延は表示上の演出。
- Q: 終局条件(確定)? → A: 盤面が埋まるか、両陣営に合法手(挟み線を作る手)がなくなったら終局(no-op手も合法手として数える)。
- Q: 終局時のcontest石の処理? → A: メーター方向で確定(ちょうど50なら元の色)して色数で勝敗。同数なら引き分け。
- Q: 1ゲームの想定プレイ時間? → A: 時間上限なし(終局条件で自然に終わる)。
- Q: UI表示情報? → A: 数字(メーター値)+CDバー(残り秒)+反転予定の予測表示(予測反転陣営+予測秒数、現在の正味圧力が一定と仮した計算)。
- Q: undo/restart? → A: restartのみ(undoなし)。
- Q: 終局画面の表示? → A: 色数+contest石のメーター内訳。
- Q: UIの言語? → A: 日本語。
- Q: 起動時の流れ? → A: タイトル/メニュー画面(ルール説明+スタートボタン)→ゲーム。
- Q: ウィンドウサイズ? → A: リサイズ可能。
- Q: 効果音? → A: 簡易(反転時・CD解錠時の音)。

## Game Definition (interview-derived, normative)

### Placement

- Standard Othello placement: a stone may be placed only on an empty square that creates at least one sandwich line (opponent stones between the placed stone and an existing own stone in one of the 8 directions).
- Each camp has its own independent cooldown (CD = 5 seconds fixed). When a camp's CD expires, that camp may place one stone on a legal square. A camp with no legal square passes when its CD expires.
- Human = black, places by click whenever CD is ready (CD starts at placement). AI = white, deterministic (same board → same move, 2-move lookahead), places with a 0.5–2 s presentation delay.
- Processing order when both camps' CDs expire in the same second: black first, then white. No placement collisions.
- Sandwich lines made only of fixed (already-flipped) stones are legal no-op moves (no pressure produced).

### Tension Meter Capture

- A stone sandwiched at placement time enters a contested state with a tension meter T in [0, 100] (white camp = 0, black camp = 100), starting at 50 (neutral).
- Contested state is entered only at placement time; flips never create new contested stones.
- Pressure per active sandwiching line: `p = ceil(N / k)`, where N = chain length (more sandwiched stones → stronger push = acceleration) and k = distance from the nearest own stone at that moment (farther → weaker push). N and k are recomputed every second.
- Every second, each contested stone's meter moves by (sum of black-line pressures − sum of white-line pressures). Lines of the same camp are summed. The meter advances every second whenever active lines exist, even if nobody places.
- A line is active while its far end holds a stone of the line's camp; if the far-end stone flips to the other camp, the line breaks (same as rescue).
- When the meter reaches 0 or 100, the stone flips to that camp. Flipped stones are fixed forever (never re-contested).
- Rescue: if all sandwiching lines on a contested stone are broken, the meter freezes (contest remains); pressure resumes when the stone is re-sandwiched.
- Contested stones count as own-camp stones (their original camp) for forming new sandwich lines.

### End and Scoring

- Game ends when the board is full, or when neither camp has any legal move (no-op moves count as legal moves). No time limit.
- At end, contested stones are resolved by meter direction (toward black end = black, toward white end = white; exactly 50 = original camp). Winner by stone count; tie = draw.

### Presentation

- UI language: Japanese. Window: resizable.
- Title/menu screen with rules explanation and start button → game.
- Contested stones display their meter value as a number.
- CD bar (remaining seconds) is displayed for each camp.
- Each contested stone shows a prediction: predicted flip camp + predicted seconds, assuming current net pressure continues (remaining distance / net pressure per second).
- End screen shows stone counts plus the meter breakdown of contested stones.
- Restart is available; no undo.
- Simple sound effects: on flip and on CD unlock.

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Play a full game against the AI (Priority: P1)

The human (black) plays the new Othello against the AI (white) in real time: place stones on legal squares when CD is ready, watch contested stones' meters tug-of-war every second, see stones flip when a meter tips to an end, and reach the end screen with the result.

**Why this priority**: Core of the product — without a playable game there is nothing.

**Independent Test**: Start a game, place stones, let meters advance, observe at least one flip, reach the end condition, see winner/score.

**Acceptance Scenarios**:

1. **Given** a new game (standard 4-stone start, both CDs ready), **When** black places a stone creating a sandwich, **Then** the sandwiched stone enters contested at meter 50 and the meter advances every second by net pressure.
2. **Given** a contested stone with only black lines pressing, **When** its meter reaches 100, **Then** the stone flips to black and becomes fixed.
3. **Given** a contested stone pressed by both camps, **When** net pressure is toward white, **Then** the meter decreases each second (tug-of-war).
4. **Given** a contested stone whose all lines are broken, **When** seconds pass, **Then** its meter freezes until it is re-sandwiched.
5. **Given** neither camp has a legal move, **When** the game state is evaluated, **Then** the game ends, contested stones are resolved by meter direction (50 = original camp), and the winner/draw is shown with the meter breakdown.

---

### User Story 2 - Understand the novelty (Priority: P2)

A new player can learn what makes this Othello different (tension meter, real-time CD, rescue, fixed flips) without external documentation.

**Why this priority**: A "never-before-seen" game fails if players cannot grasp its rules.

**Independent Test**: A first-time player can state the core novel mechanic after an in-app explanation.

**Acceptance Scenarios**:

1. **Given** a first-time player, **When** they open the game, **Then** the title screen shows a Japanese explanation of the novel rules (meter 0–100, per-second pressure, flip at the ends, rescue = freeze).

---

### User Story 3 - Read the board state at a glance (Priority: P3)

During play, the player can read each contested stone's meter value, each camp's CD remaining, and the predicted flip (camp + seconds) per contested stone.

**Why this priority**: Real-time tug-of-war is unplayable without visible state.

**Independent Test**: With a contested stone on the board, the displayed number equals the internal meter value, and the prediction equals remaining distance / net pressure.

**Acceptance Scenarios**:

1. **Given** a contested stone at meter 62 with net black pressure 4/s, **When** the UI renders, **Then** it shows 62 and a prediction like "黒 約10秒".
2. **Given** black's CD has 3 seconds remaining, **When** the UI renders, **Then** black's CD bar shows 3 s.

---

### User Story 4 - Restart (Priority: P4)

The player can restart the game at any time from a fresh standard start.

**Why this priority**: Real-time games can be lost to accidents; restart is the only recovery (no undo).

**Independent Test**: Trigger restart mid-game; board returns to the 4-stone start with both CDs ready.

**Acceptance Scenarios**:

1. **Given** a game in progress, **When** the player triggers restart, **Then** the board, meters, and CDs reset to initial state.

---

### Edge Cases

- Both camps' CDs expire in the same second → black processed first, then white; no collisions.
- A camp's CD expires but it has no legal square → pass (skip).
- No-op move (sandwich of fixed stones only) → legal, no pressure, counts as a legal move for end detection.
- Meter exactly 50 at game end → counted as original camp.
- Rescue (all lines broken) → meter frozen, contest retained; re-sandwich resumes pressure.
- A line's far-end stone flips to the other camp → that line breaks (rescue-equivalent).
- Flip at exactly 0 or 100 → flips immediately at that second.
- Draw at end (equal counts after contested resolution) → draw shown.
- N and k recomputed every second; a stone's pressure can change as the board evolves.
- Game can end with contested stones still on the board.

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: System MUST implement placement per standard Othello (sandwich-line legal squares only) on a standard 8x8 board with the standard 4-stone start.
- **FR-002**: System MUST run in real time with independent per-camp cooldowns of 5 seconds; a camp may place one stone per expired CD; a camp with no legal square passes.
- **FR-003**: System MUST assign contested state (meter 0–100, start 50) only to stones sandwiched at placement time.
- **FR-004**: System MUST advance each contested stone's meter every second by the net sum of active line pressures (same-camp lines summed; camps opposed as difference), recomputing N and k each second.
- **FR-005**: System MUST flip a contested stone to the camp whose end (0 or 100) its meter reaches; flipped stones are permanently fixed.
- **FR-006**: System MUST freeze the meter of a contested stone whose all sandwiching lines are broken (including via far-end flip), and resume pressure when re-sandwiched.
- **FR-007**: System MUST treat contested stones as own-camp stones for line formation.
- **FR-008**: System MUST provide an AI opponent (white) that is deterministic with 2-move lookahead and places with a 0.5–2 s presentation delay; the human plays black.
- **FR-009**: System MUST end the game when the board is full or neither camp has a legal move, resolve contested stones by meter direction (50 = original camp), count stones, and show winner or draw.
- **FR-010**: System MUST display per contested stone: meter value as a number and predicted flip camp + seconds (current net pressure assumed constant); and per camp: CD remaining as a bar.
- **FR-011**: System MUST show a Japanese rules explanation on a title/menu screen with a start button.
- **FR-012**: System MUST support restart (no undo).
- **FR-013**: System MUST deliver as a Windows executable (.exe) built with C++ and DxLib, with a resizable window.
- **FR-014**: System MUST process same-second CD expiries in fixed order black → white (deterministic, testable).
- **FR-015**: System MUST play simple sound effects on flip and on CD unlock.

### Key Entities

- **Stone**: position, camp (black/white), state (normal / contested / fixed), original camp; contested stones carry a meter value.
- **Tension Meter**: value 0–100 per contested stone; 0 = white end, 100 = black end, start 50.
- **Sandwich Line**: camp, chain length N, distance k (recomputed each second), active while the far end holds a stone of the line's camp.
- **Cooldown**: per-camp timer, 5 seconds fixed.
- **Game**: board state, per-camp CD, end condition, score (counts + meter breakdown), result (win/draw).

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: A player can complete a full game in one session without external help.
- **SC-002**: A standard-Othello player can name the core novel mechanic (tension meter tug-of-war) and its strategic effect (delayed, count-dependent, distance-dependent flips) after playing one game.
- **SC-003**: The engine is deterministic: the same board and the same AI decisions reproduce identical meter values and flips in automated tests.
- **SC-004**: A contested stone's displayed number and prediction match the internal state at every render (verified by test).
- **SC-005**: A full game against the AI completes end-to-end (start → flips → end screen) without manual intervention.

## Assumptions

- The project is greenfield (no existing game code in the repo).
- Human vs AI only in v1; no local human-vs-human, no online multiplayer.
- The AI's 0.5–2 s delay is presentation only; decision logic is deterministic.
- Meter values are integers; pressure p = ceil(N/k) is at least 1 per active line.
- No time limit per game; games end by the natural end condition.
- DxLib is available on the development machine (Windows).
