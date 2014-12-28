#!/usr/bin/perl
# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.

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


#
#  Copyright Statement:
#  --------------------
#  This software is protected by Copyright and the information contained
#  herein is confidential. The software may not be copied and the information
#  contained herein may not be used or disclosed except with the written
#  permission of MediaTek Inc. (C) 2009
#
#  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
#  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
#  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
#  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
#  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
#  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
#  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
#  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
#  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
#  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
#  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
#  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
#
#  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
#  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
#  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
#  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
#  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
#
#  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
#  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
#  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
#  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
#  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
#
#
($#ARGV != 0) && &Usage;
$filename = $ARGV[0];

die "Can't find the file: $filename\n" if (!-e $filename);
die "Should be a file, not a folder: $filename\n" if (-d $filename);
exit 0 if (-z $filename);

$pmPath = "mediatek/build/tools";
$pmFile = "yusuLib.pm";
$i = 0;
while (!-e "${pmPath}/${pmFile}") {
  $i++;
  $pmPath= "../$pmPath";
  last if ($i > 8);
}
die "Can NOT find $pmPath" if (!-e $pmPath);

unshift(@INC, $pmPath);
require $pmFile;

$showthefile = -1;
$send_admin_notify = 0;

  open(F, $filename ) || die "Can't open $filename";

  while(<F>) {
    if (/In file included from (\S+)/) { # appear beofre depend header error
      $thefile = $1;
      if ($thefile =~ /([^:]+)/) {
        $thefile = $1;
      } else {
        &SendAdmEmail(0x4, $thefile);
        # SendAdmEmail is for exception that out of our expectation
      }
      $showthefile = 0;
      $errlist{$thefile} .= $_;
      #print "$thefile\n";
    } elsif (/^\s+ from (\S+)/) {
      # this line should be appeard after "In file included from"
      $errlist{$thefile} .= $_;
      #print "$thefile\n";
    } elsif (/(\S+):\d+: undefined reference /) { # link error
      print "$_";
      $thefile = $1;
      $errlist{$thefile} .= $_;
      $errfilelist{$thefile} = 1;
    } elsif (/(\S+) error:/) { # error in .c/.h
      $depfile = $1;
      $errModule++;
      print "$_";

      if ($depfile =~ /([^:]+)/){
        $depfile = $1;
        if (!defined($errfilelist{$depfile})) {
           $errfilelist{$depfile} = 2;
        }
      } else {
        &SendAdmEmail(0x4, $_);
      }

      if ($depfile =~ /\.h/) {
        $errlist{$thefile} .= $_;
      } elsif (($depfile =~ /([^\/]+\.)c/) || ($depfile =~ /([^\/]+\.)cpp/))  {
        $objfile = $1 . "o";
        $errlist{$depfile} .= $_;
        $showthefile = 1;
      }

      # the following part should be reviewed
      if ($showthefile < 0) {
        print "\n\n$thefile:\n";
        $showthefile = 1;
      } elsif ($showthefile == 0) {
        print "\n\n$thefile:\n";
        $showthefile = 1;
        $errfilelist{$thefile} = 1;
      }
      if ($errModule > 300) {
        &SendAdmEmail(0x8, $errModule);
        last;
      }
    } elsif (/(\S+): Permission denied/) {
      &SendAdmEmail(0x2, $_); # need to check permission or codebase
      $perlist{$1} = 1;
      print "$_";
    } elsif (/No such file or directory/) { # may copy fully.
      &SendAdmEmail(0x10, $_);
      print "$_";
    } elsif (/No rule to make target/) { # Android.mk define the file that is not exist in P4
      &SendAdmEmail(0x40, $_);
      print "$_";
    } elsif (/directory does not exist/) { # java related
      &SendAdmEmail(0x1, $_);
      print "$_";
    } elsif (/\[(\S+)\] Error/) { # how to map Java ???
      if (!(($objfile ne "") && (/$objfile/))) { 
        # Skip if obj file error since we have dealt this part.
        &SendAdmEmail(0x1, $_);
        print "$_";
      }
    }
    $preLine = $_;
  } ## while
  close(F);

exit 0;
     
sub Usage {
  warn << "__END_OF_USAGE";
Usage:  parse_err.pl SOURCE_LOG_FILE
Parse SOURCE_LOG_FILE and extract error infomation to stdout.

SOURCE_LOG_FILE is the file from stdout while compiling uboot, kernel,
or Android system.
__END_OF_USAGE

  exit 1;
}
