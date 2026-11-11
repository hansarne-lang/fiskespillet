#include "Economy/Betting/FiskBetting.h"
#include "FiskCareerProfile.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
namespace
{
    int64 BetAmount(const TArray<FFiskBetPayout>& Out, const FGuid& Bettor)
    { const FFiskBetPayout* P = Out.FindByPredicate([&](const FFiskBetPayout& V) { return V.Bettor == Bettor; }); return P ? P->Amount : -1; }
    int64 BetSum(const TArray<FFiskBetPayout>& Out) { int64 S = 0; for (const auto& P : Out) { S += P.Amount; } return S; }
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskBettingExamplesTest,"Fiskelaget.Economy.BettingExamples",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskBettingExamplesTest::RunTest(const FString& Parameters)
{
    // The five worked examples in Kravspesifikasjon v1.1, "Avklaring 8. november 2026 - veddemål".
    const FGuid A = FGuid::NewGuid(), B = FGuid::NewGuid(), C = FGuid::NewGuid(), X = FGuid::NewGuid(), Y = FGuid::NewGuid(), Z = FGuid::NewGuid();
    {
        FFiskBetBook Book;
        Book.Register(FGuid::NewGuid(), A, X, 25000); Book.Register(FGuid::NewGuid(), B, X, 5000); Book.Register(FGuid::NewGuid(), C, Y, 10000);
        const auto Out = Book.Settle({{X, 30000}, {Y, 12000}});
        TestEqual(TEXT("1: pot"), Book.Pot(), 40000LL);
        TestEqual(TEXT("1: A gets 333.34"), BetAmount(Out, A), 33334LL);
        TestEqual(TEXT("1: B gets 66.66"), BetAmount(Out, B), 6666LL);
        TestEqual(TEXT("1: C gets nothing"), BetAmount(Out, C), 0LL);
        TestEqual(TEXT("1: whole pot paid"), BetSum(Out), 40000LL);
    }
    {
        FFiskBetBook Book;
        Book.Register(FGuid::NewGuid(), A, X, 10000); Book.Register(FGuid::NewGuid(), C, Y, 10000); Book.Register(FGuid::NewGuid(), B, Z, 10000);
        const auto Out = Book.Settle({{X, 28200}, {Y, 28200}, {Z, 9000}});
        TestEqual(TEXT("2: A 150"), BetAmount(Out, A), 15000LL);
        TestEqual(TEXT("2: C 150"), BetAmount(Out, C), 15000LL);
        TestEqual(TEXT("2: B 0"), BetAmount(Out, B), 0LL);
    }
    {
        FFiskBetBook Book;
        Book.Register(FGuid::NewGuid(), A, X, 25000); Book.Register(FGuid::NewGuid(), B, Y, 5000);
        const auto Out = Book.Settle({{X, 5000}, {Y, 7000}, {Z, 40000}});
        TestTrue(TEXT("3: no correct tip refunds everyone"), BetAmount(Out, A) == 25000 && BetAmount(Out, B) == 5000 && Out[0].bRefund);
    }
    {
        // 4: X arrived 17:00:05, so the caller leaves X's catch out. Y's fish decides.
        FFiskBetBook Book;
        Book.Register(FGuid::NewGuid(), A, X, 10000); Book.Register(FGuid::NewGuid(), B, Y, 10000);
        TestEqual(TEXT("4: Y wins the whole pot"), BetAmount(Book.Settle({{Y, 6000}}), B), 20000LL);
        const auto None = Book.Settle({});
        TestTrue(TEXT("4: nobody delivered in time refunds"), BetAmount(None, A) == 10000 && BetAmount(None, B) == 10000);
    }
    {
        // 5: the payout receipt is derived from the bet ID, so settling again after a restart credits once.
        FFiskBetBook Book; const FGuid Bet = FGuid::NewGuid();
        Book.Register(Bet, A, X, 10000); Book.Register(FGuid::NewGuid(), B, Y, 25000);
        FFiskCareerProfile Career = FFiskCareerProfile::Create(TEXT("BAR"));
        const int64 Before = Career.Cash;
        for (int32 Run = 0; Run < 2; ++Run)
        {
            for (const FFiskBetPayout& P : Book.Settle({{X, 9000}})) { if (P.Bettor == A && P.Amount > 0) { TestTrue(TEXT("5: credit accepted"), Career.Credit(P.Receipt, P.Amount)); } }
        }
        TestEqual(TEXT("5: paid once"), Career.Cash, Before + 35000);
    }
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskBettingRulesTest,"Fiskelaget.Economy.BettingRules",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskBettingRulesTest::RunTest(const FString& Parameters)
{
    const FGuid A = FGuid::NewGuid(), B = FGuid::NewGuid(), X = FGuid::NewGuid();
    FFiskBetBook Book; const FGuid Bet = FGuid::NewGuid();
    TestTrue(TEXT("Only $50, $100 or $250"), Book.Register(FGuid::NewGuid(), A, X, 7500) == EFiskBetResult::InvalidStake);
    TestTrue(TEXT("Register"), Book.Register(Bet, A, X, 10000) == EFiskBetResult::Registered);
    TestTrue(TEXT("Retry is not charged twice"), Book.Register(Bet, A, X, 10000) == EFiskBetResult::AlreadyRegistered);
    TestTrue(TEXT("Same ID, other content"), Book.Register(Bet, A, X, 25000) == EFiskBetResult::Conflict);
    TestTrue(TEXT("One bet per player per day"), Book.Register(FGuid::NewGuid(), A, X, 5000) == EFiskBetResult::AlreadyBetToday);
    TestTrue(TEXT("No bet on yourself"), Book.Register(FGuid::NewGuid(), X, X, 5000) == EFiskBetResult::SelfBet);
    TestEqual(TEXT("Pot holds one stake"), Book.Pot(), 10000LL);
    Book.Close();
    TestTrue(TEXT("Closed at 17:00"), Book.Register(FGuid::NewGuid(), B, X, 5000) == EFiskBetResult::Closed);
    TestTrue(TEXT("Retry after closing is still recognised"), Book.Register(Bet, A, X, 10000) == EFiskBetResult::AlreadyRegistered);
    TestTrue(TEXT("Empty book pays nothing"), FFiskBetBook().Settle({}).IsEmpty());
    for (int32 I = 0; I < 3; ++I) { TestEqual(TEXT("Bar menu offers the book's stakes"), FFiskCareerProfile::BetStake(I), FFiskBetBook::Stakes[I]); }
    TestEqual(TEXT("No fourth stake"), FFiskCareerProfile::BetStake(3), -1LL);

    // Rule 7: equal largest stakes give the leftover cent to the earliest bet.
    FFiskBetBook Even; const FGuid P1 = FGuid::NewGuid(), P2 = FGuid::NewGuid(), P3 = FGuid::NewGuid(), Q = FGuid::NewGuid(), R = FGuid::NewGuid();
    Even.Register(FGuid::NewGuid(), P1, Q, 5000); Even.Register(FGuid::NewGuid(), P2, Q, 5000); Even.Register(FGuid::NewGuid(), P3, Q, 5000);
    Even.Register(FGuid::NewGuid(), FGuid::NewGuid(), R, 10000);
    const auto Out = Even.Settle({{Q, 20000}, {R, 10000}});
    TestEqual(TEXT("Remainder to earliest"), Out[0].Amount, 8334LL);
    TestTrue(TEXT("Others rounded down"), Out[1].Amount == 8333 && Out[2].Amount == 8333 && Out[3].Amount == 0);

    // The pot is always paid out exactly, for many random books.
    FRandomStream Random(77);
    for (int32 Round = 0; Round < 200; ++Round)
    {
        FFiskBetBook Random12; TArray<FGuid> Fishers; for (int32 I = 0; I < 4; ++I) { Fishers.Add(FGuid::NewGuid()); }
        for (int32 I = 0; I < 12; ++I) { Random12.Register(FGuid::NewGuid(), FGuid::NewGuid(), Fishers[Random.RandRange(0, 3)], FFiskBetBook::Stakes[Random.RandRange(0, 2)]); }
        TArray<FFiskBetCatch> Catches; for (int32 I = 0; I < 4; ++I) { if (Random.RandRange(0, 2)) { Catches.Add({Fishers[I], Random.RandRange(1, 4) * 1000}); } }
        const auto Paid = Random12.Settle(Catches);
        int64 S = 0; bool bNegative = false; for (const auto& P : Paid) { S += P.Amount; bNegative |= P.Amount < 0; }
        if (S != Random12.Pot() || bNegative) { AddError(FString::Printf(TEXT("Pot not conserved in round %d"), Round)); return false; }
    }
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskBettingCountingTest,"Fiskelaget.Economy.BettingCountingCatches",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskBettingCountingTest::RunTest(const FString& Parameters)
{
    const FGuid X = FGuid::NewGuid(), Y = FGuid::NewGuid(), Shore = FGuid::NewGuid();
    // Positive grams: delivered in time. Negative grams: the same fish weight, but not delivered in time.
    auto Row = [](int32 Player, const TArray<int32>& Fish)
    {
        FFiskDayResult R; R.PlayerId = Player;
        for (const int32 Weight : Fish) { FFiskCatchResult C; C.Fish.WeightGrams = FMath::Abs(Weight); C.bDelivered = Weight > 0; R.Catches.Add(C); }
        return R;
    };
    // X reconnected during the day (two player IDs, one career). Y's biggest fish came in too late.
    const TArray<FFiskDayResult> Rows = { Row(1, {4000, -9000}), Row(2, {-12000, 3000}), Row(3, {6500}), Row(4, {50000}) };
    const TMap<int32, FGuid> Careers = {{1, X}, {2, Y}, {3, X}, {4, Shore}};
    const TArray<FFiskBetCatch> Out = FFiskBetBook::CountingCatches(Rows, Careers, TSet<FGuid>{X, Y});
    auto Grams = [&](const FGuid& Fisher) { const FFiskBetCatch* C = Out.FindByPredicate([&](const FFiskBetCatch& V) { return V.Fisher == Fisher; }); return C ? C->Grams : -1; };
    TestEqual(TEXT("Largest delivered fish across the career's player IDs"), Grams(X), 6500);
    TestEqual(TEXT("A late fish does not count"), Grams(Y), 3000);
    TestEqual(TEXT("A player who was ashore is not compared"), Grams(Shore), -1);
    TestEqual(TEXT("Unbound rows are ignored"), FFiskBetBook::CountingCatches(Rows, {}, TSet<FGuid>{X}).Num(), 0);
    TestTrue(TEXT("No delivered fish leaves the fisher out"), FFiskBetBook::CountingCatches({Row(1, {-9000})}, Careers, TSet<FGuid>{X}).IsEmpty());
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskBettingSummaryTest,"Fiskelaget.Economy.BettingBarSummary",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskBettingSummaryTest::RunTest(const FString& Parameters)
{
    const FGuid A = FGuid::NewGuid(), B = FGuid::NewGuid(), C = FGuid::NewGuid(), X = FGuid::NewGuid(), Y = FGuid::NewGuid();
    FFiskBetBook Book;
    Book.Register(FGuid::NewGuid(), A, X, 10000); Book.Register(FGuid::NewGuid(), B, X, 5000); Book.Register(FGuid::NewGuid(), C, Y, 25000);
    const TArray<FFiskBetCatch> Counting = {{X, 9100}, {Y, 4200}};
    const FFiskBetSummary S = FFiskBetBook::Summarize(Book.GetBets(), Book.Settle(Counting), Counting);
    TestEqual(TEXT("Pot"), S.Pot, 40000LL); TestEqual(TEXT("Bets"), S.Bets, 3); TestFalse(TEXT("Not a refund"), S.bRefund);
    TestEqual(TEXT("Two winning bettors"), S.Winners, 2); TestTrue(TEXT("Winning fisher"), S.WinningFishers.Num() == 1 && S.WinningFishers[0] == X);
    TestEqual(TEXT("Winning weight"), S.WinningGrams, 9100);
    const FFiskBetSummary R = FFiskBetBook::Summarize(Book.GetBets(), Book.Settle({}), {});
    TestTrue(TEXT("No counting catch is a refund without winners"), R.bRefund && R.Winners == 0 && R.WinningFishers.IsEmpty() && R.Pot == 40000);
    const FFiskBetSummary E = FFiskBetBook::Summarize({}, {}, {});
    TestTrue(TEXT("No bets"), E.Bets == 0 && E.Pot == 0 && !E.bRefund);
    return true;
}
#endif
