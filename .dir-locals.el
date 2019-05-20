((nil . ((eval . (setq
                  cmake-ide-project-dir "/Users/kspark/local/src/ImageFeatureDetector/"
                  cmake-ide-build-dir "/Users/kspark/local/src/ImageFeatureDetector/build/"
                  projectile-project-test-cmd #'helm-ctest
                  projectile-project-compilation-cmd #'helm-make-projectile
                  projectile-project-compilation-dir '(cmake-ide-build-dir)
                  helm-make-build-dir '(cmake-ide-build-dir)
                  helm-ctest-dir '(cmake-ide-build-dir)
                  ))
         (projectile-project-name . "imagefeaturedetector")
         ;;         (projectile-project-run-cmd . "./run.sh")
         (projectile-project-configure-cmd . "cmake -DCMAKE_BUILD_TYPE=Debug
                  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..")
         (helm-make-arguments . "-j4"))))
