import smtplib
import os
import sys
from email.mime.text import MIMEText


def send_email(recipient, message):
    sender = os.getenv("EMAIL_SENDER")  
    password = os.getenv("EMAIL_PASSWORD")

    server = smtplib.SMTP("smtp.gmail.com", 587)
    server.starttls()

    try:
        server.login(sender, password)
        msg = MIMEText(message)
        msg["Subject"] = "CLICK ME PLEASE!"
        server.sendmail(sender, recipient, msg.as_string())


        return "The message was sent successfully!"
    except Exception as _ex:
        return f"{_ex}\nCheck your login or password please!"


def main():
    if len(sys.argv) != 3:
        print("Usage: python send.py <recipient_email> <message>")
        sys.exit(1)
    recipient_email = sys.argv[1]
    message = sys.argv[2]
    result = send_email(recipient_email, message)
    print(result)

if __name__ == "__main__":
    main()