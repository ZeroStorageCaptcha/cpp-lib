# Zero storage captcha C++ headers lib

Offline captcha without any file system or database storage.

Dependencies: Qt5.

## Inspiration

Captcha is required in many public projects to protect against spammers and similar unwanted activity.

As practice shows, developers are accustomed to using external services even to use fonts (!). 
Obviously, implementing bot protection is more complicated than storing fonts or style sheets locally, 
which is why most developers turn to suck-to-free corporations.

The goal of Zero storage captcha is to make it easy to use a locally generated captcha picture without having to store the answer. 
This technology allows any project to have high-quality and ethical captcha without spending VPS disk space.

## How it works

When generating a captcha, the user receives a picture and a token. 
The token is a cryptographic key to verify the correctness of the answer. It is created based on:

1) Captcha answer
2) Hash MD5
3) Session unique string
4) Time marker

The time marker (time token) changes every one and a half minutes, due to which the same answer after 
a few minutes has a completely new token to check the truth.

The user, along with the picture, must provide a verification token, which he will report to the server along with the response to the picture. 
This can be implemented both through javascript and when generating html pages using the templating method.

The system remembers the previous time token in order to ensure the correct perception of the captcha generated 
a few seconds before the time token change.

Due to this architecture, the lifetime of each captcha ranges from 1.5 to 3 minutes, 
after which the verification token will always show failure.

To make it impossible to use one captcha twice, the used verification token gets into a special cache, 
where it is stored for several minutes of the life cycle of the token. 
The token is considered used after the first validation check.

Session signing key generated at every start of app (at first call to ZeroStorageCaptchaCrypto::KeyHolder class).

Check `examples` folder to see C++ interface or if your project not in C++, 
you can use Zero storage captcha as separate cross-platform local [service](https://github.com/ZeroStorageCaptcha/api-daemon).
