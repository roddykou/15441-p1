################################################################################
# Makefile                                                                     #
#                                                                              #
# Description: This file contains the make rules for Recitation 1.             #
#                                                                              #
# Authors: Athula Balachandran <abalacha@cs.cmu.edu>,                          #
#          Wolf Richter <wolf@cs.cmu.edu>                                      #
#                                                                              #
################################################################################

default: echo_server echo_client

echo_server:
	@gcc echo_server.c -g -o echo_server -Wall -Werror

echo_client:
	@gcc echo_client.c -g -o echo_client -Wall -Werror

clean:
	@rm -f echo_server echo_client
