#include "World/FiskSpeciesArt.h"
#include "World/FiskHarbourSound.h"
#include "FiskHarbourRules.h"
#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskAllSpeciesMesh,"Fiskelaget.LivingHarbour.FifteenValidSpeciesMeshes",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FFiskAllSpeciesMesh::RunTest(const FString&)
{ for(int32 I=0;I<15;++I) { const auto B=FiskSpeciesArt::Body(I),F=FiskSpeciesArt::Fins(I);TestTrue(TEXT("Closed finite body and normals"),B.Valid());TestTrue(TEXT("Finite double-sided fins"),F.Valid());TestTrue(TEXT("Bounded geometry"),B.Vertices.Num()<1000 && F.Vertices.Num()<1000); }return true; }
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskSpeciesDistinct,"Fiskelaget.LivingHarbour.DistinctSpeciesSilhouettes",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FFiskSpeciesDistinct::RunTest(const FString&)
{ TSet<FString> Shapes;for(int32 I=0;I<15;++I) { const auto* P=FiskSpeciesArt::Find(I);Shapes.Add(FString::Printf(TEXT("%.0f %.0f %.0f %.0f %.0f"),P->Length,P->Width,P->Height,P->Dorsal,P->Bill)); }TestEqual(TEXT("All fifteen have distinct shape definitions"),Shapes.Num(),15);return true; }
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskBillSail,"Fiskelaget.LivingHarbour.BillSailAndEyeFeatures",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FFiskBillSail::RunTest(const FString&)
{ TestTrue(TEXT("Swordfish has longest bill"),FiskSpeciesArt::Find(13)->Bill>FiskSpeciesArt::Find(6)->Bill);TestTrue(TEXT("Sailfish sail exceeds body height"),FiskSpeciesArt::Find(12)->Dorsal>FiskSpeciesArt::Find(12)->Height*2);TestTrue(TEXT("Bigeye eye exceeds yellowfin"),FiskSpeciesArt::Find(11)->Eye>FiskSpeciesArt::Find(2)->Eye);TestTrue(TEXT("Permit is taller than wide"),FiskSpeciesArt::Find(9)->Height>FiskSpeciesArt::Find(9)->Width*4);return true; }
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskArtBounds,"Fiskelaget.LivingHarbour.SafeCatalogueBounds",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FFiskArtBounds::RunTest(const FString&)
{ TestNull(TEXT("Negative selection rejected"),FiskSpeciesArt::Find(-1));TestNull(TEXT("Index fifteen rejected"),FiskSpeciesArt::Find(15));TestTrue(TEXT("Invalid geometry is empty"),FiskSpeciesArt::Body(999).Vertices.IsEmpty());for(int32 I=0;I<15;++I) { const auto* P=FiskSpeciesArt::Find(I);TestTrue(TEXT("Palette is finite"),FMath::IsFinite(P->Body.R) && FMath::IsFinite(P->Body.G) && FMath::IsFinite(P->Body.B) && FMath::IsFinite(P->Accent.R) && FMath::IsFinite(P->Accent.G) && FMath::IsFinite(P->Accent.B));TestTrue(TEXT("Eyes have opposite sides"),FiskSpeciesArt::Eye(I,-1).Y<0 && FiskSpeciesArt::Eye(I,1).Y>0); }return true; }
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskPresenceAuth,"Fiskelaget.LivingHarbour.PresenceAuthority",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FFiskPresenceAuth::RunTest(const FString&)
{ for(int32 P=0;P<3;++P) { TestTrue(TEXT("Ready own dock choice accepted"),FiskHarbour::CanChoose(true,true,true,P));TestFalse(TEXT("Foreign profile rejected"),FiskHarbour::CanChoose(true,false,true,P));TestFalse(TEXT("At sea rejected"),FiskHarbour::CanChoose(true,true,false,P));TestFalse(TEXT("Unready rejected"),FiskHarbour::CanChoose(false,true,true,P)); }TestFalse(TEXT("Client cannot self-declare sea state"),FiskHarbour::CanChoose(true,true,true,3));return true; }
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskPresenceDerived,"Fiskelaget.LivingHarbour.SeaAndDisconnectOverridePresence",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FFiskPresenceDerived::RunTest(const FString&)
{ for(int32 P=0;P<3;++P) { TestEqual(TEXT("Server trip wins"),FiskHarbour::PublicPlace(true,true,P),3);TestEqual(TEXT("Disconnected hidden"),FiskHarbour::PublicPlace(false,false,P),-1);TestEqual(TEXT("Dock choice restored"),FiskHarbour::PublicPlace(true,false,P),P); }return true; }
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskMeetingPlaces,"Fiskelaget.LivingHarbour.TwelveDistinctMeetingPlaces",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FFiskMeetingPlaces::RunTest(const FString&)
{ for(int32 I=0;I<12;++I) { for(int32 J=0;J<I;++J) { TestTrue(TEXT("Bar people have separate anchors"),FVector::Dist(FiskHarbour::MeetingPoint(I,2),FiskHarbour::MeetingPoint(J,2))>=300);TestTrue(TEXT("Marina people have separate anchors"),FVector::Dist(FiskHarbour::MeetingPoint(I,0),FiskHarbour::MeetingPoint(J,0))>=260); } }return true; }
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskHarbourSamples,"Fiskelaget.LivingHarbour.OriginalAudioSamples",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FFiskHarbourSamples::RunTest(const FString&)
{ double MusicEnergy=0,AmbientEnergy=0,Difference=0;for(uint64 I=0;I<24000*8;++I) { const double A=FiskHarbourAudio::Sample(I,true),B=FiskHarbourAudio::Sample(I,false);if(!FMath::IsFinite(A) || !FMath::IsFinite(B) || FMath::Abs(A)>.4 || FMath::Abs(B)>.1) { AddError(TEXT("Audio sample out of bounds"));return false; }MusicEnergy+=A*A;AmbientEnergy+=B*B;Difference+=FMath::Abs(A-B); }TestTrue(TEXT("Music non-silent"),MusicEnergy>1);TestTrue(TEXT("Ambience retained with music off"),AmbientEnergy>1);TestTrue(TEXT("Music toggle changes audio"),Difference>1);return true; }
#endif
