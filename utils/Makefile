.PHONY : all prerequisites mosquitto linter libnyoci clean

all : # TODO.

mosquitto: # Hard-coded test to make easier on my dev system.
	./make_diffs.sh -d ~/git/TARGETS/mosquitto -f with_tls

linter:
	g++ check_braces.cpp -o linter

libnyoci:
	sh ~/git/libnyoci/configure --enable-code-coverage --enable-embedded
	
prerequisites:
	sudo ../setup.sh

clean :
	rm -rf coverage_files* cov_files_* *.dot diff_* linter a.out
