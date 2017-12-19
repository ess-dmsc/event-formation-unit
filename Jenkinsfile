project = "efu"

def failure_function(exception_obj, failureMessage) {
    def toEmails = [[$class: 'DevelopersRecipientProvider']]
    emailext body: '${DEFAULT_CONTENT}\n\"' + failureMessage + '\"\n\nCheck console output at $BUILD_URL to view the results.', recipientProviders: toEmails, subject: '${DEFAULT_SUBJECT}'
    slackSend color: 'danger', message: "event-formation-unit: " + failureMessage
    throw exception_obj
}

def get_osx_pipeline()
{
    return {
        stage("MacOSX") {
            node ("macos") {
            // Delete workspace when build is done
                cleanWs()

                dir("${project}/code") {
                    try {
                        checkout scm
                    } catch (e) {
                        failure_function(e, 'MacOSX / Checkout failed')
                    }
                }

                dir("${project}/build") {
                    try {
                        sh "cmake ../code"
                    } catch (e) {
                        failure_function(e, 'MacOSX / CMake failed')
                    }

                    try {
                        sh "make runtest"
                    } catch (e) {
                        failure_function(e, 'MacOSX / build failed')
                    }
                }

            }
        }
    }
}


node('kafka-client && centos7') {
    cleanWs()

    def builders = [:]

    builders['MocOSX'] = get_osx_pipeline()

    parallel builders

    
    dir("code") {
        stage("Checkout") {
            try {
                checkout scm
            } catch (e) {
                failure_function(e, 'Checkout failed')
            }
        }

        stage("Static analysis") {
            try {
                sh "cloc --by-file --xml --out=cloc.xml ."
                sh "xsltproc jenkins/cloc2sloccount.xsl cloc.xml > sloccount.sc"
                sloccountPublish encoding: '', pattern: ''
            } catch (e) {
                failure_function(e, 'Static analysis failed')
            }
        }
    }


    dir("build") {
        stage("Build") {
            try {
                sh "cmake -DCOV=1 ../code"
                sh "make -j 5"
            } catch (e) {
                failure_function(e, 'Failed to compile')
            }
        }

        stage("Run tests") {
            try {
                sh "make -j 5 runtest"
                junit 'test_results/*.xml'
                sh "make coverage_xml"
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
                    zoomCoverageChart: false
                ])
            } catch (e) {
                junit 'test_results/*.xml'
                failure_function(e, 'Unit tests failed')
            }
        }
    }
}
