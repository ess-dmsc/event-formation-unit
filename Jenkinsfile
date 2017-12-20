def failure_function(exception_obj, failureMessage) {
    def toEmails = [[$class: 'DevelopersRecipientProvider']]
    emailext body: '${DEFAULT_CONTENT}\n\"' + failureMessage + '\"\n\nCheck console output at $BUILD_URL to view the results.', recipientProviders: toEmails, subject: '${DEFAULT_SUBJECT}'
    slackSend color: 'danger', message: "event-formation-unit: " + failureMessage
    throw exception_obj
}

node('kafka-client && centos7') {
    cleanWs()

    dir("code") {
        try {
            stage("Checkout projects") {
                checkout scm
            }
        } catch (e) {
            failure_function(e, 'Checkout failed')
        }
    }
    dir("build") {
        try {
            stage("Run CMake") {
                sh "cmake ../code"
            }
        } catch (e) {
            failure_function(e, 'CMake failed')
        }

        try {
            stage("Build everything") {
                sh "make"
            }
        } catch (e) {
            failure_function(e, 'Failed to compile')
        }

        try {
            stage("Run unit tests") {
                sh "make runtest"
            }
        } catch (e) {
            failure_function(e, 'Unit tests failed')
        }
    }

    stage("Coverage") {

        dir("build") {
            try {
                sh "cmake -DCOV=1 ../code"
            } catch (e) {
                failure_function(e, 'cmake COV failed')
            }

            try {
                sh "make runtest"
                junit 'test_results/*test.xml'
                sh "make coverage"
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
                failure_function(e, 'generate coverage failed')
                junit 'test/unit_tests_run.xml'
            }
        }
    }
}
