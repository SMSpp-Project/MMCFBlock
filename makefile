##############################################################################
################################ makefile ####################################
##############################################################################
#                                                                            #
#   makefile of MMCFBlock                                                    #
#                                                                            #
#   Note that $(SMS++INC) is assumed to include any -I directive             #
#   corresponding to external libraries needed by SMS++, at least to the     #
#   extent in which they are needed by the parts of SMS++ used by MMCFBlock. #
#                                                                            #
#   Input:  $(CC)       = compiler command                                   #
#           $(SW)       = compiler options                                   #
#           $(SMS++INC) = the -I$( core SMS++ directory )                    #
#           $(SMS++OBJ) = the libSMS++ library itself                        #
#           $(MCFBkINC) = the -I$( MCFBlock directory )                      #
#           $(MCFBkH)   = the .h files of MCFBlock                           #
#           $(BKBkINC)  = the -I$( BinaryKnapsackBlock directory )           #
#           $(BKBkH)    = the .h files of BinaryKnapsackBlock                #
#           $(MMCFSDR)  = the directory where the source is                  #
#                                                                            #
#   Output: $(MMCFOBJ)  = the final object(s) / library                      #
#           $(MMCFH)    = the .h files to include                            #
#           $(MMCFINC)  = the -I$( source directory )                        #
#                                                                            #
#                              Antonio Frangioni                             #
#                         Dipartimento di Informatica                        #
#                             Universita' di Pisa                            #
#                                                                            #
##############################################################################


# macros to be exported - - - - - - - - - - - - - - - - - - - - - - - - - - -

MMCFOBJ = $(MMCFSDR)/obj/MMCFBlock.o \
	$(MMCFSDR)/obj/MMCFNetworkDesignBlock.o

MMCFINC = -I$(MMCFSDR)/include

MMCFH   = $(MMCFSDR)/include/MMCFBlock.h \
	$(MMCFSDR)/include/MMCFNetworkDesignBlock.h

# the C++20 module smspp.mmcfblock - - - - - - - - - - - - - - - - - - - - -
# [see makefile-mod of SMS++]

$(eval $(call SMSppMODrule,MMCF,$(MMCFSDR),MMCFBlock,smspp.mmcfblock,\
	BKBk MCFBk))

# clean - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

clean::
	rm -f $(MMCFOBJ) $(MILPBSDR)/*~

# dependencies: every .o from its .cpp + every recursively included .h- - - -

$(MMCFSDR)/obj/MMCFBlock.o: $(MMCFSDR)/src/MMCFBlock.cpp \
	$(MMCFSDR)/include/MMCFBlock.h $(MCFBkH) $(BKBkH) $(SMS++OBJ)
	$(CC) -c $(MMCFSDR)/src/MMCFBlock.cpp -o $@ $(MMCFINC) $(MCFBkINC) \
	$(BKBkINC) $(SMS++INC) $(SW)

$(MMCFSDR)/obj/MMCFNetworkDesignBlock.o: \
	$(MMCFSDR)/src/MMCFNetworkDesignBlock.cpp \
	$(MMCFSDR)/include/MMCFNetworkDesignBlock.h \
	$(MMCFSDR)/include/MMCFBlock.h $(MCFBkH) $(BKBkH) $(SMS++OBJ)
	$(CC) -c $(MMCFSDR)/src/MMCFNetworkDesignBlock.cpp -o $@ $(MMCFINC) \
	$(MCFBkINC) $(BKBkINC) $(SMS++INC) $(SW)

########################## End of makefile ###################################
