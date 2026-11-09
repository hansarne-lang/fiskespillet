#pragma once
#include "CoreMinimal.h"

// Beach bar bets (#22), rules 1-8 in DECISIONS.md (2026-11-08). Pure server-side model: no UI, network
// or saving yet. Money is in cents, like FFiskCareerProfile::Cash. People are career profile IDs, so a
// bet survives a disconnect (rule 8). Which catches count (delivered in time, rod owner, which fishers
// are compared) is decided by the caller; this model only splits the pot.

enum class EFiskBetResult : uint8 { Registered, AlreadyRegistered, InvalidStake, AlreadyBetToday, SelfBet, Closed, Conflict };

struct FFiskBet
{
    FGuid Id;          // Unique per bet; also the receipt ID for its payout (rule 8)
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
    FGuid Receipt;     // = FFiskBet::Id
    FGuid Bettor;
    int64 Amount = 0;  // Cents; 0 for a losing bet
    bool bRefund = false;
};

class FFiskBetBook
{
public:
    static constexpr int64 Stakes[3] = {5000, 10000, 25000};  // $50, $100, $250 (rule 1)
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
private:
    TArray<FFiskBet> Bets;
    bool bClosed = false;
};
