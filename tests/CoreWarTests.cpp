#include <QtTest/QtTest>

#include <cstdio>
#include <fstream>

#include "src/VirtualMachine.hpp"
#include "src/Tokenizer.hpp"
#include "src/Assembler.hpp"

class CoreWarTests : public QObject
{
	Q_OBJECT

private slots:
	void VM_coreZeroSizeException();
	void VM_tooManyInstrException();
	void VM_duplicatedPlayerException();

	void TOK_noTokenException();
	void TOK_readTokens();
	void TOK_assignNewText();
	void TOK_keptSeparators();
	void TOK_toLowerCase();

	void ASM_correctWarrior1();
	void ASM_correctWarrior2();
	void ASM_wrongInvalidInstruction();
	void ASM_wrongInvalidLabel();
	void ASM_wrongRepeatedLabel();
};

void CoreWarTests::VM_coreZeroSizeException()
{
	QVERIFY_EXCEPTION_THROWN(VirtualMachine::Core(0), std::invalid_argument);
}

void CoreWarTests::VM_tooManyInstrException()
{
	VirtualMachine vm;

	std::vector<VirtualMachine::Core::Instruction> vec(10000);

	QVERIFY_EXCEPTION_THROWN(vm.loadProgram(vec, 0), std::invalid_argument);
}

void CoreWarTests::VM_duplicatedPlayerException()
{
	VirtualMachine vm;

	std::vector<VirtualMachine::Core::Instruction> vec(300);

	vm.loadProgram(vec, 0);

	QVERIFY_EXCEPTION_THROWN(vm.loadProgram(vec, 0), std::runtime_error);

	vm.reset();

	vm.loadProgram(vec, 0, false);

	QVERIFY_EXCEPTION_THROWN(vm.loadProgram(vec, 0, false), std::runtime_error);
}

void CoreWarTests::TOK_noTokenException()
{
	Tokenizer t(std::string(), "");

	QCOMPARE(t.isToken(), false);
	QVERIFY_EXCEPTION_THROWN(t.next(), std::out_of_range);
}

void CoreWarTests::TOK_readTokens()
{
	Tokenizer t("Abcd:Efgh", ":");

	QCOMPARE(t.next(), std::string("Abcd"));
	QCOMPARE(t.next(), std::string("Efgh"));
}

void CoreWarTests::TOK_assignNewText()
{
	Tokenizer t("abcd:ef", ":");

	QCOMPARE(t.next(), std::string("abcd"));

	t.assign(std::string("ghijkl:mn"));

	QCOMPARE(t.next(), std::string("ghijkl"));
}

void CoreWarTests::TOK_keptSeparators()
{
	Tokenizer t("abcd.efgh 12, 13", " .", ",");

	QCOMPARE(t.next(), std::string("abcd"));
	QCOMPARE(t.next(), std::string("efgh"));
	QCOMPARE(t.next(), std::string("12"));
	QCOMPARE(t.next(), std::string(","));
	QCOMPARE(t.next(), std::string("13"));
}

void CoreWarTests::TOK_toLowerCase()
{
	Tokenizer t("ABcDeFgHiJkLMNopqrstuVwxYZ", "", "", true);

	QCOMPARE(t.next(), std::string("abcdefghijklmnopqrstuvwxyz"));
}

void CoreWarTests::ASM_correctWarrior1()
{
	const char* name = std::tmpnam(NULL);

	std::ofstream out(name);

	out << "mov.i 2, 0\nkil 0, 0\nkil 10, 10";

	out.close();

	Assembler::getInstance().openFile(name);

	QCOMPARE(Assembler::getInstance().assembly(), true);

	remove(name);
}

void CoreWarTests::ASM_correctWarrior2()
{
	const char* name = std::tmpnam(NULL);

	std::ofstream out(name);

	out << "mov.b 1, 2\njmp 2, #-2\nkil #0, #2\njmp @-1\nkil #0, #0";

	out.close();

	Assembler::getInstance().openFile(name);

	QCOMPARE(Assembler::getInstance().assembly(), true);

	remove(name);
}

void CoreWarTests::ASM_wrongInvalidInstruction()
{
	const char* name = std::tmpnam(NULL);

	std::ofstream out(name);

	out << "addiu $1, $2";

	out.close();

	Assembler::getInstance().openFile(name);

	QCOMPARE(Assembler::getInstance().assembly(), false);

	remove(name);
}

void CoreWarTests::ASM_wrongInvalidLabel()
{
	const char* name = std::tmpnam(NULL);

	std::ofstream out(name);

	out << "ab: sub 20, 30\njmp abc";

	out.close();

	Assembler::getInstance().openFile(name);

	QCOMPARE(Assembler::getInstance().assembly(), false);

	remove(name);
}

void CoreWarTests::ASM_wrongRepeatedLabel()
{
	const char* name = std::tmpnam(NULL);

	std::ofstream out(name);

	out << "abc: sub 20, 30\nabc: add -1, 1";

	out.close();

	Assembler::getInstance().openFile(name);

	QCOMPARE(Assembler::getInstance().assembly(), false);

	remove(name);
}



QTEST_MAIN(CoreWarTests)
#include "CoreWarTests.moc"
