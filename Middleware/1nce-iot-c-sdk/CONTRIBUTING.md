
#  Contributing Guidelines

Thank you for your interest in 1NCE IoT C SDK! You can help improve 1NCE IoT C SDK by reporting issues ( bugs, feature requests, correction, or additional documentation). we greatly value feedback and contributions from our community.

## General Overview
If you are just beginning to work with SDK you might first read our documentation.

## General Tips

From experience, the following recommendations help to get a software contribution into SDK main faster: 

* **Keep it simple:**  Try to use what is already there and don't change existing functions if not absolutely necessary.

*  **Keep it small:** A PR with >1000 lines of changes will very likely make even the most active reviewer put your review on their long to-do list.
* **Keep it modular:**  Make extensions to a feature or new features for a platform optionally to use.

* **Provide tests:** They should be comprehensible and easy to be executed. Alternatively comprehensive testing procedures should be provided with your pull request.

##  Feature Requests

We welcome you to use the GitHub issue tracker to report bugs or suggest features.

Before opening a new feature request, check the [existing feature requests](https://github.com/1NCE-GmbH/1nce-iot-c-sdk/issues) if there's one already open on the same topic.

To request new features or enhancements, just open a new [feature request issue](https://github.com/1NCE-GmbH/1nce-iot-c-sdk/issues/new). 

Describe your use case

* Why you need this feature?

* Why this feature is important for 1NCE SDK?

## Bug Reports

While bugs are unfortunate, nothing is perfect in Software. We can't fix what we don't know about, so feel free to report bugs even if you are not sure is a bug or not. 

Before reporting a bug, have a look at [open bugs](https://github.com/1NCE-GmbH/1nce-iot-c-sdk/labels/bug). Maybe someone has already reported your error.

Once you have verified that the bug you have found hasn't been reported, opening an issue with bug label. 

Each bug report issue have 5 sections that are to help us and other contributors undersand the issue and eventually reproduce it: 
1. description
2. Steps to reproduce the issue
3. Expected results
4. Actual results
5.  Anything unusual about your environment or integration

In summary, try to include as much information as possible to help maintainers or other developers fix the bug quickly.


##  Contributor License Agreements

We may ask you to sign a [Contributor License Agreement (CLA)](https://en.wikipedia.org/wiki/Contributor_License_Agreement) for larger changes.


##  Contributing via Pull Requests

Contributions via pull requests are much appreciated. Before sending us a pull request, please ensure that:

1. You are working against the latest source on the *main* branch.

2. You check existing open, and recently merged, pull requests to make sure someone else hasn't addressed the problem already.

3. You open an issue to discuss any significant work - we would hate for your time to be wasted.

To send us a pull request, please:

4. Fork the repository.

5. Modify the source; please focus on the specific change you are contributing. If you also reformat all the code, it will be hard for us to focus on your change.

6. Ensure that your contributions conform to the [style guide](./docs/doxygen/style.dox).

7. Ensure local tests pass.

8. Commit to your fork using clear commit messages.

9. Send us a pull request, answering any default questions in the pull request interface.

10. Pay attention to any automated CI failures reported in the pull request, and stay involved in the conversation.

GitHub provides additional document on [forking a repository](https://help.github.com/articles/fork-a-repo/) and

[creating a pull request](https://help.github.com/articles/creating-a-pull-request/).

##  Finding contributions to work on

Looking at the existing issues is a great way to find something to contribute on. As our projects, by default, use the default GitHub issue labels ((enhancement/bug/duplicate/help wanted/invalid/question/wontfix), looking at any ['help wanted'](https://github.com/1NCE-GmbH/1nce-iot-c-sdk/labels/help%20wanted) issues is a great place to start.

## Documentation Contribution


Documentation improvements are always welcome and a good starting point for new contributors. This kind of contribution is merged quite quickly in general.

1NCE SDK documentation is built with [doxygen][doxygen]. Doxygen is configured to parse header (.h) and `doc.txt` files in the 1NCE SDK source code to generate the modules and packages documentation.

General documentation pages are written in Markdown.

##  Security issue notifications

If you discover a potential security issue in this project we ask that you notify 1NCE via our email cert@1nce.com. Please do **not** create a public github issue. We would appreciate waiting for a period before reporting it on public, to allow us to release the fix.


##  Licensing

See the [LICENSE](./LICENSE) file for our project's licensing. We will ask you to confirm the licensing of your contribution.
