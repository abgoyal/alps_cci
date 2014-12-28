#!usr/bin/perl
# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.
#
# MediaTek Inc. (C) 2010. All rights reserved.
#
# BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
# THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
# RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
# AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
# NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
# SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
# SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
# THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
# THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
# CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
# SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
# STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
# CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
# AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
# OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
# MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
#
# The following software/firmware and/or related documentation ("MediaTek Software")
# have been modified by MediaTek Inc. All revisions are subject to any receiver's
# applicable license agreements with MediaTek Inc.



my $configFile = $ARGV[0];
my $optrConfigFile = $ARGV[1];
die "the file $configFile is NOT exsit\n" if ( ! -e $configFile);
die "the file $optrConfigFile is NOT exsit\n" if ( (! -e $optrConfigFile) && ($optrConfigFile ne "") );
open FILE, "<$configFile";
my %config_for;
while (<FILE>) {
	if (/^(\S+)\s*=\s*(\S+)/) {
		$config_for{$1} = $2;
	}
}
close FILE;
open FILE, "<$optrConfigFile";
while (<FILE>) {
        if (/^(\S+)\s*=\s*(\S+)/) {
                $config_for{$1} = $2;
        }
}
close FILE;


my $filedir = "mediatek/frameworks/featureoption/java/com/mediatek/featureoption";
my $write_filename = "$filedir/FeatureOption.java";
my $input_file = "mediatek/build/tools/javaoption.pm";
open(INPUT,$input_file) or die "can not open $input_file:$!\n";
my %javaoption;
while(<INPUT>)
{
	chomp;
	next if(/^\#/);
	next if(/^\s*$/);
	if(/\s*(\w+)\s*/)
	{
                if ($javaoption{$1} == 1)
                {   
                        die "$1 already define in $input_file";
                } else {
                        push (@need_options,$1);
                        $javaoption{$1} = 1;
                }   
        }   
}


system("chmod u+w $write_filename") if (-e $write_filename);
system("mkdir -p $filedir") if ( ! -d "$filedir");
die "can NOT open $write_filename:$!" if ( ! open OUT_FILE, ">$write_filename");
print OUT_FILE "\/* generated by mediatek *\/\n\n";
print OUT_FILE "package com.mediatek.featureoption;\n";
print OUT_FILE "\npublic final class FeatureOption\n{\n";
foreach my $option (@need_options) {
	if ($config_for{$option} eq "yes") {
		&gen_java_file ($write_filename, $option, "true");
	}
	elsif ($config_for{$option} eq "no"){
		&gen_java_file ($write_filename, $option, "false");
	}
	else
	{
		print "\"$option\" not match\n";
	}
}
print OUT_FILE "}\n";
close OUT_FILE;
sub gen_java_file {
	my ($filename, $option, $value) = @_;
	print OUT_FILE "    /**\n     * check if $option is turned on or not\n     */\n";
	if ( $option eq "GEMINI") {
		print OUT_FILE "    public static final boolean MTK_${option}_SUPPORT = $value;\n";
		print "public static final boolean MTK_${option}_SUPPORT = $value\n";
	}
	else {
		print OUT_FILE "    public static final boolean ${option} = $value;\n";
		print "public static final boolean ${option} = $value\n";
	}
}