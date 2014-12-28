#!/usr/bin/perl -w
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



use strict;
use File::Copy;
use Tie::File;

&usage if (($#ARGV == 0) && ($ARGV[0] =~ /^-h|-help|--help$/));
&usage if ($#ARGV < 3);

my ($projectMF, $defaultConfigFile, $projectConfigFile, $optChgLog, $DEBUG) = @ARGV;

# disable debug mode in default
$DEBUG = 0 if (!defined $DEBUG);

# rule mapping table between feature option & kernel configuration
my $FeatOpt2KernCfg = 
{
  MTK_ATV_CHIP      => {
                          MTK_MT5192   => ['CONFIG_MATV_DRIVER=y'],
                          NONE         => ['# CONFIG_MATV_DRIVER is not set']
                       },

#
# add new mapping rule here
#
  OPTEND            => {
                          TRUE         => ['CONFIG_OPTEND21=m', 'CONFIG_OPTEND22=y', 'CONFIG_MATV_DRIVER=y'],
                          FALSE        => ['# CONFIG_OPTEND21 is not set', '# CONFIG_OPTEND22 is not set']
                       }
};

my $FeatOpt2KernCfgRelMode = 
{
 # add release mode feature mapping rule here 
};


unlink($optChgLog) if (-e $optChgLog);

# generate updated kernel config. file
my $projectFeatureConfig = readProjMF($projectMF);
my $projectKernelConfig  = getProjectConfig($projectFeatureConfig, $FeatOpt2KernCfg);
writeProjectConfig($defaultConfigFile, $projectConfigFile, $projectKernelConfig);


# **************************************************************************
# subroutines
# **************************************************************************

# read project configuration file
sub readProjMF
{
  my $mf = shift;
  my $features = {};
  open(MF, "<$mf") or die "can NOT open $mf!\n";
  while (<MF>)
  {
    if (/^(\S+)\s*=\s*(\S+)/)
    {
      $features->{$1} = $2;
    }
  }
  close(MF);
  return $features;
}

# get customized kernel options
sub getProjectConfig
{
  my ($projectFeature, $mappingTable) = @_;
  my @projectConfig = ();
  foreach my $opt (keys %$projectFeature)
  {
    if (exists $mappingTable->{$opt}
     && exists $mappingTable->{$opt}->{$projectFeature->{$opt}}
       )
    {
      foreach my $item (@{$mappingTable->{$opt}->{$projectFeature->{$opt}}})
      {
        if (chkMappingTblValidity($item, \@projectConfig))
        {# check mapping table validity
          warn "Duplicated kernel config. option introduced by \"$opt=$projectFeature->{$opt}\" ==> \"$item\"\n";
        }
        else
        {# collect current project's kernel config. options
          push(@projectConfig, $item);
        }
      }
    }
  }
  return \@projectConfig;
}

# write kernel .config file
sub writeProjectConfig
{
  my ($defaultConfig, $kernelConfig, $projectConfig_ref) = @_;
  my @array = ();
  my $optChgCount = 0;

  copy($defaultConfig, $kernelConfig) or die "Copy file from \"$defaultConfig\" to \"$kernelConfig\" failed!\n";
  tie @array, 'Tie::File', $kernelConfig or die "can NOT open file \"$kernelConfig\"!\n";
  foreach my $opt (@array)
  {
    if ($opt =~ /^\s*#\s*(CONFIG_\w+)\s+is\s+not\s+set/)
    {# disabled options
      my $curSetting = $opt;
      my $newSetting = "";
      my $optName    = $1;

      if ($newSetting = [grep(/^\s*$optName\s*=.*/,@$projectConfig_ref)]->[0])
      {# enable options
        $opt = $newSetting;
        $optChgCount++;
        print "[Option Changed] \"$curSetting\" ==> \"$newSetting\"\n" if ($DEBUG);
        LOG("[Option Changed] \"$curSetting\" ==> \"$newSetting\"\n", $optChgLog);
        # remove changed option setting from mapping table
        rmElem($newSetting, $projectConfig_ref);
        print "\$projectConfig_ref:\n",join("\n",@$projectConfig_ref),"\n" if ($DEBUG);
      }
      elsif (grep(/^\s*#\s*$optName\s+is\s+not\s+set/,@$projectConfig_ref))
      {# remove no changed option setting from mapping table
        rmElem($curSetting, $projectConfig_ref);
      }
      else
      {
        next;
      }
    }
    elsif ($opt =~ /^\s*(CONFIG_\w+)\s*=\s*(.*)/)
    {# enabled options
      my $curSetting = $opt;
      my $newSetting = "";
      my $optName    = $1;
      my $curOptVal  = $2;

      if ($newSetting = [grep(/^\s*#\s*$optName\s+is\s+not\s+set/,@$projectConfig_ref)]->[0])
      {# disable options
        $opt = $newSetting;
        $optChgCount++;
        print "[Option Changed] \"$curSetting\" ==> \"$newSetting\"\n" if ($DEBUG);
        LOG("[Option Changed] \"$curSetting\" ==> \"$newSetting\"\n", $optChgLog);
        # remove changed option setting from mapping table
        rmElem($newSetting, $projectConfig_ref);
        print "\$projectConfig_ref:\n",join("\n",@$projectConfig_ref),"\n" if ($DEBUG);
      }
      elsif ($newSetting = [grep(/^\s*$optName\s*=.*/,@$projectConfig_ref)]->[0])
      {# change options' value if not the default setting
        my $newOptVal = "";
        $newOptVal = $1 if ($newSetting =~ /=\s*(.*)/);
        if ($newOptVal ne $curOptVal)
        {
          $opt = $newSetting;
          $optChgCount++;
          print "[Option Changed] \"$curSetting\" ==> \"$newSetting\"\n" if ($DEBUG);
          LOG("[Option Changed] \"$curSetting\" ==> \"$newSetting\"\n", $optChgLog);
          # remove changed option setting from mapping table
          rmElem($newSetting, $projectConfig_ref);
          print "\$projectConfig_ref:\n",join("\n",@$projectConfig_ref),"\n" if ($DEBUG);
        }
        else
        {# remove no changed option setting from mapping table
          rmElem($curSetting, $projectConfig_ref);
        }
      }
      else
      {
        next;
      }
    }
    else
    {
      next;
    }
  }

  # add new options' setting
  if (@$projectConfig_ref)
  {
    push(@array, @$projectConfig_ref);
    $optChgCount++;
    print "[Option Added]\n",join("\n",@$projectConfig_ref),"\n" if ($DEBUG);
    LOG("[Option Added]\n".join("\n",@$projectConfig_ref)."\n", $optChgLog);
  }
  untie @array;

  if (!$optChgCount)
  {
    print "No changes in kernel .config file!\n";
  }
  else
  {
    print "Update kernel .config file DONE!\n";
  }
}

# check if there are duplicated options
sub chkMappingTblValidity
{
  my ($option, $projectConfig_ref) = @_;
  my $tmp = quotemeta($option);
  return grep(/$tmp/, @$projectConfig_ref);
}

# remove an element from an array
sub rmElem
{
  my ($element, $arr_ref) = @_;
  $element = quotemeta($element);
  @$arr_ref = grep(!/^$element$/i, @$arr_ref);
}

# log changes
sub LOG
{
  my ($msg, $file) = @_;
  open(LOGFILE, ">>$file") or die "can NOT open \"$file\"!\n";
  print LOGFILE $msg;
  close(LOGFILE);
}

sub usage
{
  print << "__EOFUSAGE";
usage: $0 projectMF defaultConfigFile projectConfig optChgLog [DEBUG]

projectMF             project makefile
defaultConfigFile     default kernel config. file 
                      (a superset of all project's kernel config options,
                      such as, kernel/default.config)
projectConfigFile     project kernel config. file
optChgLog             log file for option changes
DEBUG                 enable switch (1/0), disabled in default
                      1: enable debug mode
                      0: disable debug mode
__EOFUSAGE
  exit 1;
}

