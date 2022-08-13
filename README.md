# Zero storage captcha C++ headers lib

Offline captcha without any file system or database storage.

Dependencies: Qt5.

## Inspiration

Captcha is required in many public projects to protect against spammers and similar automated unwanted activity.

As practice shows, developers are accustomed to using external services even to use fonts (!). 
Obviously, implementing bot protection is more complicated than storing fonts or style sheets locally, 
which is why most developers turn to suck-to-free corporations.

The goal of Zero Storage Captcha is to make it easy to use a locally generated captcha picture without having to store the answer. 
This technology allows any project to have high-quality and ethical captcha without spending VPS disk space (CPU only yep).

## How it works

When generating a captcha, the user receives a picture and a token. 
The token is a string key to verify the correctness of the answer. It is created based on:

BASE64( MD5_HASH( CAPTCHA_ANSWER + TIME_TOKEN + CAPTCHA_ID + SESSION_KEY ) ) + "_" + CAPTCHA_ID

- TIME_TOKEN - temporary marker for limiting captcha life circle;
- CAPTCHA_ID - size_t validation key for concrete captcha;
- SESSION_KEY - random run-time session string for unique hash value.

Regular captcha token looks like this: `QyhnRNJolLJxnJaSqzQVww_1`.

The user, along with the picture, must provide a verification token, which he will report to the server along with the response to the picture. 
This can be implemented both through javascript and when generating html pages using the templating method.

The system remembers the previous time token in order to ensure the correct perception of the captcha generated 
a few seconds before the time token change.

Due to this architecture, the lifetime of each captcha ranges from 1.5 to 3 minutes, 
after which the verification token will always show failure.

To make it impossible to use one captcha twice, the used verification captcha id gets into a special cache, 
where it is stored for several minutes of the life cycle of the concrete captcha token. 
The token is considered used after the first validation check. Storing captcha id is very cheap: the id has a weight of 8 bytes (for a 64-bit system). For example, to store a million solved captchas at one time would need less than 8 MB of RAM.

To guarantee the uniqueness of captcha tokens, when generating the first captcha, creates a SESSION_KEY (random string), which takes part in creating the control hash that forms the token. For the generation of the hash is chosen MD5, as it is a productive algorithm, which is sufficiently reliable in the context of Zero Storage Captcha.

Check `examples` folder to see C++ interface or if your project not in C++, 
you can use Zero storage captcha as separate cross-platform local [service](https://github.com/ZeroStorageCaptcha/api-daemon).
