#pragma once
#include "CoreMinimal.h"
#include "Economy/FiskDayLedger.h"
#include "FiskCareerProfile.h"

// Beach bar bets (#22), rules 1-8 in DECISIONS.md (2026-11-08). Pure model; the server (Bootstrap)
// owns one book per day. Money is in cents, like FFiskCareerProfile::Cash. People are career profile
// IDs, so a bet survives a disconnect (rule 8). CountingCatches picks the catches that are compared
// (decision 2026-11-10); Settle only splits the pot.

enum class EFiskBetResult : uint8 { Registered, AlreadyRegistered, InvalidStake, AlreadyBetToday, SelfBet, Closed, Conflict,
    NotAshore, FisherNotAtSea, NotCharged };   // Checked by the server before the book (step 2)

struct FFiskBet
{
    FGuid Id;          // Unique per bet; the stake receipt ID. The payout uses FFiskCareerProfile::BetPayoutId(Id) (rule 8)
    FGuid Bettor;
    FGuid Fisher;
    int64 Stake = 0;   // Cents
    int32 Order = 0;   // Registration order, used for the rounding remainder (rule 7)
};

struct FFiskBetCatch
{
    FGuid Fisher;
    int32 Grams = 0;
};

struct FFiskBetPayout
{
    FGuid Receipt;     // = FFiskCareerProfile::BetPayoutId(FFiskBet::Id)
    FGuid Bettor;
    int64 Amount = 0;  // Cents; 0 for a losing bet
    bool bRefund = false;
};

// The day's public result for the bar board, shown after 17:00 (#18): pot, how many bet, whether all
// stakes went back, which fishers won and with how many grams, and how many bettors won.
struct FFiskBetSummary
{
    int64 Pot = 0;
    int32 Bets = 0;
    bool bRefund = false;
    TArray<FGuid> WinningFishers;
    int32 WinningGrams = 0;
    int32 Winners = 0;
};

class FFiskBetBook
{
public:
    static constexpr int64 Stakes[3] = {5000, 10000, 25000};  // $50, $100, $250 (rule 1); same as FFiskCareerProfile::BetStake
    static bool ValidStake(int64 Cents) { return Cents == Stakes[0] || Cents == Stakes[1] || Cents == Stakes[2]; }

    // Rule 1 and 2. Retrying the same bet (same ID and content) is AlreadyRegistered and must not be
    // charged again; the same ID with other content is a Conflict.
    EFiskBetResult Register(const FGuid& Id, const FGuid& Bettor, const FGuid& Fisher, int64 Stake);
    void Close() { bClosed = true; }   // At 17:00 no new bets are accepted
    bool IsClosed() const { return bClosed; }
    int64 Pot() const;
    const TArray<FFiskBet>& GetBets() const { return Bets; }

    // Rules 3-7. One payout per bet; the amounts always add up to the pot.
    TArray<FFiskBetPayout> Settle(const TArray<FFiskBetCatch>& Counting) const;

    // Decision 2026-11-10: the largest fish delivered in time (bDelivered; the ledger books a fish on
    // its rod owner) for each fisher who was out at sea while bets could be registered. Rows are
    // matched to careers through the day's player-to-career binding; a fisher without a delivered
    // fish is left out.
    static FFiskBetSummary Summarize(const TArray<FFiskBet>& Bets, const TArray<FFiskBetPayout>& Payouts, const TArray<FFiskBetCatch>& Counting);
    static TArray<FFiskBetCatch> CountingCatches(const TArray<FFiskDayResult>& Rows, const TMap<int32, FGuid>& Careers, const TSet<FGuid>& OutAtSea);
private:
    TArray<FFiskBet> Bets;
    bool bClosed = false;
};
