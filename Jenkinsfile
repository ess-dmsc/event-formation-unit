@Library('ecdc-pipeline')
import ecdcpipeline.ContainerBuildNode
import ecdcpipeline.PipelineBuilder

project = "event-formation-unit"
module_src="${project}/src/modules/"
coverage_on = "centos7"
clangformat_os = "debian10"
archive_what = "centos7-release"

// Set number of old builds to keep.
 properties([[
     $class: 'BuildDiscarderProperty',
     strategy: [
         $class: 'LogRotator',
         artifactDaysToKeepStr: '',
         artifactNumToKeepStr: '10',
         daysToKeepStr: '',
         numToKeepStr: ''
     ]
 ]]);

container_build_nodes = [
  'centos7': ContainerBuildNode.getDefaultContainerBuildNode('centos7-gcc8'),
  'centos7-release': ContainerBuildNode.getDefaultContainerBuildNode('centos7-gcc8'),
  'debian10': ContainerBuildNode.getDefaultContainerBuildNode('debian10'),
  'ubuntu2004': ContainerBuildNode.getDefaultContainerBuildNode('ubuntu2004')
]

def error_messages = []
def failure_function(exception_obj, failureMessage) {
    def emailmap = [ "mortenjc@jcaps.com":"morten.christensen@ess.eu", \
                     "jenny.walker@live.co.uk":"jennifer.walker@ess.eu", \
                     "28659574+amues@users.noreply.github.com": "afonso.mukai@ess.eu"]

    COMMITEMAIL = sh (
        script: 'git --no-pager show -s --format=\'%ae\' 2> /dev/null || echo "none"',
        returnStdout: true
    ).trim()

    COMMITNAME = sh (
        script: 'git --no-pager show -s --format=\'%an\' 2> /dev/null || echo "none"',
        returnStdout: true
    ).trim()

    EXTRATEXT="uninitialized"
    TOMAIL='no email'
    if (!COMMITEMAIL.contains("ess.eu")) {
      if (emailmap.containsKey(COMMITEMAIL)) {
         EXTRATEXT="non ess.eu - found in mail map"
         TOMAIL= emailmap.get(COMMITEMAIL)
      } else {
         EXTRATEXT="non ess.eu - not found in mail map"
         TOMAIL='morten.christensen@ess.eu'
      }
    } else {
      EXTRATEXT="ess.eu - no lookup"
      TOMAIL=COMMITEMAIL
    }

    def toEmails = [[$class: 'DevelopersRecipientProvider']]

    emailext body: '${DEFAULT_CONTENT}\n\"' + failureMessage \
                    + '\"\n\nCheck console output at $BUILD_URL to view the results.\n\n' \
                    + 'Committer: ' + COMMITNAME + '\n' + 'Email:' + COMMITEMAIL \
                    + '\n' + EXTRATEXT + '\n mapped to: ' + TOMAIL,
            to: TOMAIL,
            subject: '${DEFAULT_SUBJECT}'
}

pipeline_builder = new PipelineBuilder(this, container_build_nodes, "/mnt/data:/home/jenkins/refdata")
pipeline_builder.activateEmailFailureNotifications()

builders = pipeline_builder.createBuilders { container ->

    pipeline_builder.stage("${container.key}: checkout") {
        dir(pipeline_builder.project) {
            scm_vars = checkout scm
        }
        // Copy source code to container
        container.copyTo(pipeline_builder.project, pipeline_builder.project)
    }  // stage


    if (container.key != clangformat_os) {
        pipeline_builder.stage("${container.key}: get dependencies") {
            container.sh """
                cd ${project}
                mkdir build
                cd build
                conan remote add --insert 0 ess-dmsc-local ${local_conan_server}
                conan install --build=outdated ..
            """
        }  // stage


        pipeline_builder.stage("${container.key}: configure") {
            def xtra_flags = ""
            if (container.key == coverage_on) {
                xtra_flags = "-DCOV=ON"
            } else if (container.key == archive_what) {
                xtra_flags = "-DCMAKE_BUILD_TYPE=Release -DCMAKE_SKIP_BUILD_RPATH=ON"
            }
            container.sh """
                cd ${project}/build
                . ./activate_run.sh
                cmake --version
                cmake -DREFDATA=/home/jenkins/refdata/EFU_reference -DCONAN=MANUAL -DGOOGLE_BENCHMARK=ON ${xtra_flags} ..
            """
        }  // stage

        pipeline_builder.stage("${container.key}: build") {
            container.sh """
                cd ${project}/build
                make --version
                make -j${pipeline_builder.numMakeJobs} all unit_tests benchmark
                cd ../utils/udpredirect
                make
            """
        }  // stage
    }

    if (container.key == clangformat_os) {
        pipeline_builder.stage("${container.key}: cppcheck") {
        try {
                def test_output = "cppcheck.txt"
                // Ignore file that crashes cppcheck
                container.sh """
                                cd ${project}
                                cppcheck --enable=all --inconclusive --template="{file},{line},{severity},{id},{message}" ./ -isrc/modules/adc_readout/test/SampleProcessingTest.cpp 2> ${test_output}
                            """
                container.copyFrom("${project}", '.')
                sh "mv -f ./${project}/* ./"
            } catch (e) {
                error_messages.push("Cppcheck step for (${container.key}) failed")
                throw e
            }
        }  // stage
        step([$class: 'WarningsPublisher', parserConfigurations: [[parserName: 'Cppcheck Parser', pattern: "cppcheck.txt"]]])
    }

    if (container.key == coverage_on) {
        pipeline_builder.stage("${container.key}: test coverage") {
            abs_dir = pwd()

            try {
                container.sh """
                        cd ${project}/build
                        . ./activate_run.sh
                        make -j${pipeline_builder.numMakeJobs} runefu
                        make coverage
                        echo skipping make -j${pipeline_builder.numMakeJobs} valgrind
                    """
                container.copyFrom("${project}", '.')
            } catch(e) {
                container.copyFrom("${project}/build/test_results", '.')
                junit 'test_results/*.xml'
                error_messages.push("Run tests (${container.key}) failed")
                throw e
            }

            dir("${project}/build") {
                    junit 'test_results/*.xml'
                    sh "../jenkins/redirect_coverage.sh ./coverage/coverage.xml ${abs_dir}/${project}"

                    step([
                        $class: 'CoberturaPublisher',
                        autoUpdateHealth: true,
                        autoUpdateStability: true,
                        coberturaReportFile: 'coverage/coverage.xml',
                        failUnhealthy: false,
                        failUnstable: false,
                        maxNumberOfBuilds: 0,
                        onlyStable: false,
                        sourceEncoding: 'ASCII',
                        zoomCoverageChart: true
                    ])
            }
        }  // stage
    } else if (container.key != clangformat_os) {
        pipeline_builder.stage("${container.key}: tests") {
            container.sh """
                cd ${project}/build
                . ./activate_run.sh
                make runtest
                make runefu
            """
        }  // stage
    }

    if (container.key == archive_what) {
        pipeline_builder.stage("${container.key}: archive") {
            container.sh """
                                mkdir -p archive/event-formation-unit
                                cp -r ${project}/build/bin archive/event-formation-unit
                                cp -r ${project}/build/generators archive/event-formation-unit
                                cp -r ${project}/build/modules archive/event-formation-unit
                                cp -r ${project}/build/lib archive/event-formation-unit
                                cp -r ${project}/build/licenses archive/event-formation-unit
                                mkdir archive/event-formation-unit/util
                                cp -r ${project}/utils/efushell archive/event-formation-unit/util
                                mkdir archive/event-formation-unit/configs
                                cp -r ${module_src}/multigrid/configs/* archive/event-formation-unit/configs/
                                cp -r ${module_src}/gdgem/configs/* archive/event-formation-unit/configs/
                                cp ${project}/utils/udpredirect/udpredirect archive/event-formation-unit/util
                                mkdir archive/event-formation-unit/data

                                # Create file with build information
                                touch archive/event-formation-unit/BUILD_INFO
                                echo 'Repository: ${project}/${env.BRANCH_NAME}' >> archive/event-formation-unit/BUILD_INFO
                                echo 'Commit: ${scm_vars.GIT_COMMIT}' >> archive/event-formation-unit/BUILD_INFO
                                echo 'Jenkins build: ${BUILD_NUMBER}' >> archive/event-formation-unit/BUILD_INFO

                                cd archive
                                tar czvf event-formation-unit-centos7.tar.gz event-formation-unit
                            """
            container.copyFrom("/home/jenkins/archive/event-formation-unit-centos7.tar.gz", '.')
            container.copyFrom("/home/jenkins/archive/event-formation-unit/BUILD_INFO", '.')
            archiveArtifacts "event-formation-unit-centos7.tar.gz,BUILD_INFO"
            if (env.CHANGE_ID) {
                // Stash archive for integration test
                stash 'event-formation-unit-centos7.tar.gz'
            }
        }
    }
}

def get_macos_pipeline()
{
    return {
        stage("macOS") {
           timestamps {
               node ("macos") {
                    // Delete workspace when build is done
                    cleanWs()

                    // temporary until all our repos have moved to using official flatbuffers and CLI11 conan packages
                    sh "conan remove -f FlatBuffers/*"
                    sh "conan remove -f cli11/*"

                    abs_dir = pwd()

                    dir("${project}") {
                        checkout scm
                    }

                    dir("${project}/build") {
                        sh "conan install --build=outdated .."
                        sh "cmake -DREFDATA=/Users/jenkins/data/EFU_reference -DCONAN=MANUAL -DCMAKE_MACOSX_RPATH=ON .."
                        sh "make -j${pipeline_builder.numCpus}"
                        sh "make -j${pipeline_builder.numCpus} unit_tests"
                        sh "make runtest"
                        sh "make runefu"
                    }
                }
            }
        }
    }
}

// Script actions start here
//
timestamps {
    node('docker') {
        dir("${project}_code") {

            stage('Checkout') {
                try {
                    scm_vars = checkout scm
                } catch (e) {
                    error_messages.push('Checkout failed')
                    throw e
                }
            }

            // skip build process if message contains '[ci skip]'
            pipeline_builder.abortBuildOnMagicCommitMessage()

            stage("Static analysis") {
                try {
                    sh "find . -name '*TestData.h' > exclude_cloc"
                    sh "find . -name 'span.hpp' >> exclude_cloc"
                    sh "find . -name 'gcovr' >> exclude_cloc"
                    sh "cloc --exclude-list-file=exclude_cloc --by-file --xml --out=cloc.xml ."
                    sh "xsltproc jenkins/cloc2sloccount.xsl cloc.xml > sloccount.sc"
                    sloccountPublish encoding: '', pattern: ''
                } catch (e) {
                    error_messages.push('Static analysis failed')
                    throw e
                }
            }
        }

        // Add macOS pipeline to builders
        builders['macOS'] = get_macos_pipeline()

        try {
            timeout(time: 2, unit: 'HOURS') {
                // run all builders in parallel
                parallel builders
            }
        } catch (e) {
            dir("${project}_code") {
              failure_function(e, error_messages.join("\n"))
            }
            throw e
        } finally {
            // Delete workspace when build is done
            cleanWs()
        }
    }
}

if (env.CHANGE_ID) {
    // This is a pull request build
    node('inttest') {
        stage('Integration Test') {
            checkout scm
            unstash 'event-formation-unit-centos7.tar.gz'
            sh "tar xzvf event-formation-unit-centos7.tar.gz"
            sh "ls -R"
            sh """
                export LD_LIBRARY_PATH=\$LD_LIBRARY_PATH:./event-formation-unit/lib/
                python3 -u ./integrationtest/integrationtest.py
            """
        }  // stage
    }  // node
}  // if
