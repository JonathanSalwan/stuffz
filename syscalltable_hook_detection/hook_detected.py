#!/usr/bin/env python2
## -*- coding: utf-8 -*-
##
##  Copyright (C) 2013 - Jonathan Salwan - http://twitter.com/JonathanSalwan
## 
##  This program is free software: you can redistribute it and/or modify
##  it under the terms of the GNU General Public License as published by
##  the Free Software Foundation, either version 3 of the License, or
##  (at your option) any later version.
## 
##  This program is distributed in the hope that it will be useful,
##  but WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##  GNU General Public License for more details.
## 
##  You should have received a copy of the GNU General Public License
##  along with this program.  If not, see <http://www.gnu.org/licenses/>.
##

import smtplib
import sys

RCPT_TO = 'mail_rcpt@gmail.com'
GUSER   = 'your_mail@gmail.com'
GPWD    = 'you_passwd'
SMTP    = 'smtp.gmail.com'

if __name__ == "__main__":

    smtpserver = smtplib.SMTP(SMTP, 587)
    smtpserver.ehlo()
    smtpserver.starttls()
    smtpserver.ehlo
    smtpserver.login(GUSER, GPWD)

    msg  = 'To: ' + RCPT_TO + '\n'
    msg += 'From: ' + GUSER + '\n'
    msg += 'Subject:Hook analyzer - Hook detected !\n\n'
    msg += 'Hook analyzer has detected a hook in syscall table '
        msg += '(syscall %s).\n' %(sys.argv[1])
        msg += 'The syscall was restored.\n\n'

    smtpserver.sendmail(GUSER, RCPT_TO, msg)
    smtpserver.close()

    sys.exit(0)
